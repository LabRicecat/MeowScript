#include "../inc/runner.hpp"
#include "../inc/tools.hpp"
#include "../inc/modules.hpp"
#include "../inc/objects.hpp"
#include "../inc/expressions.hpp"

MEOWSCRIPT_SOURCE_FILE

GeneralTypeToken MeowScript::run_file(std::string file, bool new_scope, bool save_scope, int load_idx, std::map<std::string,Variable> external_vars, fs::path from, bool pass_return_down, bool same_scope) {
    return run_text(read(file),true,save_scope,load_idx,external_vars,from,pass_return_down,same_scope);
}

GeneralTypeToken MeowScript::run_lexed(lexed_tokens lines, bool new_scope, bool save_scope, int load_idx, std::map<std::string,Variable> external_vars, fs::path from, bool pass_return_down, bool same_scope) {
    if(from != "") {
        global::include_path.push(from);
    }
    if(load_idx < 0) {
        if(!new_scope) {
            save_scope = false; // potential "memory leak" if else!
        }
        if(!same_scope) {
            MeowScript::new_scope(new_scope ? -2:-1,external_vars);
        }
    }
    else {
        load_scope(load_idx,external_vars);
        save_scope = false;
    }

    unsigned int ln = current_scope()->current_line;

    global::line_count.push(0);

    for(size_t i = 0; i < lines.size(); ++i) {
        GeneralTypeToken ret;
        global::line_count.top() = lines[i].line_count;
        current_scope()->current_line = lines[i].line_count;
        if(lines[i].source.empty()) {
            continue;
        }

        std::vector<General_type> identf_line;
        identf_line.push_back(get_type(lines[i].source[0]));
        global::add_trace(global::get_line(),tools::until_newline(lines[i].source),global::include_path.empty() ? std::filesystem::current_path().string() : global::include_path.top().string());

        while(identf_line.front() == General_type::COMPOUND) {
            lines[i].source[0] = tools::check4compound(lines[i].source[0]).to_string();
            identf_line.front() = get_type(lines[i].source[0]);
        }

        std::string name = lines[i].source[0];

        std::string str_line;
        for(auto i : lines[i].source) {
            for(size_t j = 0; j < i.content.size(); ++j) {
                if(i.content[j] == '\"') {
                    i.content.insert(i.content.end() - 2 - j,'\\');
                    ++j;
                }
            }
            if(i.in_quotes) {
                i = "\"" + i.content + "\"";
            }
            str_line += " " + i.content;
        }

        if(is_expression("(" + str_line + ")")) {
            Variable result = parse_expression("(" + str_line + ")");
            ret = result;
        }
        else if(identf_line.front() == General_type::COMMAND) {
            auto cp = lines[i].source;
            if(cp.size() != 0)
                cp.erase(cp.begin());
            Command* command = get_command_overload(name,cp);

            std::vector<GeneralTypeToken> args;
            identf_line.erase(identf_line.begin());
            if(command == nullptr) {
                std::string err_msg = "No overload of command \"" + name + "\" matches arglist!\n\t- Got: [";
                for(size_t k = 0; k < cp.size(); ++k) {
                    err_msg += (k == 0 ? "":", ") + general_t2token(get_type(cp[k])).content;
                }
                throw errors::MWSMessageException(err_msg + "]",global::get_line());
            }
            if((command->args.size() == 0 || !(command->args.back().matches(car_Ongoing))) && command->args.size() != lines[i].source.size()-1) {
                std::string err = "Too many/few arguments for command: " + command->name + "\n\t- Expected: " + std::to_string(command->args.size()) + "\n\t- But got: " + std::to_string(lines[i].source.size()-1);
                throw errors::MWSMessageException{err,global::get_line()};
            }

            bool f_ongoing = false;
            for(size_t j = 0; j < cp.size(); ++j) {
                if(f_ongoing) {
                    args.push_back(GeneralTypeToken(cp[j],command->args[j]));
                }
                else {
                    auto identf = get_type(cp[j],command->args[j]);
                    if(command->args[j].matches(car_Ongoing)) {
                        f_ongoing = true;
                        args.push_back(GeneralTypeToken(cp[j]));
                        continue;
                    }
                    if(!command->args[j].matches(identf)) {
                        std::string err_msg = "Invalid argument:\n\t- Expected: [";
                        for(size_t k = 0; k < command->args[j].carry.size(); ++k) {
                            err_msg += (k == 0 ? "":",") + general_t2token(General_type(command->args[j].carry[k]-1)).content;
                        }
                        err_msg += "]\n\t- But got: " + general_t2token(identf).content + " (" + cp[j].content + ")";
                        throw errors::MWSMessageException(err_msg,global::get_line());
                    }
                    else {
                        args.push_back(GeneralTypeToken(cp[j],command->args[j]));
                    }
                }
            }

            ret = command->run(args);
        }
        else if(identf_line.front() == General_type::FUNCTION) {
            bool shadow_return = false;
            for(size_t j = 1; j < lines[i].source.size(); ++j) {
                identf_line.push_back(get_type(lines[i].source[j]));
            }
            if(identf_line.size() > 3 || identf_line.size() < 2) {
                throw errors::MWSMessageException{"Too many/few arguments for primitiv function call!\n- Call Sytax: name(<args>)[!]",global::get_line()};
            }
            auto arglist = get_type(lines[i].source[1],General_type::ARGUMENTLIST);
            if(arglist != General_type::ARGUMENTLIST) {
                std::string err = "Unexpected token after function call:\n\t- Expected: Argumentlist\n\t- But got: " + (identf_line.size() != 1 ? general_t2token(identf_line[1]).content : "VOID");
                throw errors::MWSMessageException{err,global::get_line()};
            }
            if(identf_line.size() == 3) {
                if(identf_line[2] != General_type::OPERATOR) {
                    std::string err = "Unexpected token after function call:\n\t- Expected: \"!\"\n\t- But got: " + general_t2token(identf_line[1]).content;
                    throw errors::MWSMessageException{err,global::get_line()};
                }
                if(lines[i].source[2].content != "!") {
                    std::string err = "Unexpected token after function call:\n\t- Expected: \"!\"\n\t- But got: " + lines[i].source[2].content;
                    throw errors::MWSMessageException{err,global::get_line()};
                }
                shadow_return = true;
            }

            Function* fun = get_function(name);

            argument_list alist = tools::parse_argument_list(lines[i].source[1]);

            if(alist.size() != fun->params.size()) {
                std::string err = "Too many/few arguments for function: " + name + "\n\t- Expected: " + std::to_string(fun->params.size()) + "\n\t- But got: " + std::to_string(alist.size());
                throw errors::MWSMessageException{err,global::get_line()};
            }

            for(size_t j = 0; j < alist.size(); ++j) {
                alist[j] = tools::check4placeholder(alist[j]);
            }

            std::vector<Variable> args;
            for(size_t j = 0; j < fun->params.size(); ++j) {
                try {
                    if(fun->params[j].matches(alist[j].to_variable())) {
                        std::string err_msg = "Invalid argument:\n\t- Expected: " + var_t2token(fun->params[j].type).content + "\n\t- But got: " + var_t2token(alist[j].to_variable().type).content;
                        throw errors::MWSMessageException(err_msg,global::get_line());
                    }
                    else {
                        ++global::in_argument_list;
                        args.push_back(alist[j].to_variable());
                        --global::in_argument_list;
                    }
                }
                catch(errors::MWSMessageException& err) {
                    std::string err_msg = "Can't convert GeneralType " + general_t2token(alist[j].type).content + " to VariableType " + var_t2token(fun->params[j].type).content + " as function parameter for function: " + name;
                    throw errors::MWSMessageException{err_msg,global::get_line()};
                }
            }

            Function cpy = *fun;
            Function cpy2 = cpy;

                // TODO: this is an ugly workaroung because after
                // the line below it will only contain garbage for 
                // some reason (???)
            Variable vret = cpy2.run(args); 
            if(!cpy2.return_type.matches(vret)) {
                throw errors::MWSMessageException{"Invalid return type!\n\t- Expected: " + var_t2token(fun->return_type.type).content + "\n\t- But got: " + general_t2token(ret.type).content,global::get_line()};
            }
            ret = vret;
            if(shadow_return) {
                ret = general_null;
            }
        }
        else if(identf_line.front() == General_type::MODULE) {
            if(!get_module(name)->enabled) {
                throw errors::MWSMessageException{"Module \"" + name + "\" is not enabled!",global::get_line()};
            }
            for(size_t j = 1; j < lines[i].source.size(); ++j) {
                identf_line.push_back(get_type(lines[i].source[j]));
            }

            if(identf_line.size() < 3 || lines[i].source[1].content != ".") {
                throw errors::MWSMessageException{"Invalid module-command call!\n\t- Expected: module.command <args>...",global::get_line()};
            }
            Module* mod = get_module(name);
            identf_line.erase(identf_line.begin()); // module-name
            identf_line.erase(identf_line.begin()); // .

            Command* command = mod->get_command(lines[i].source[2].content);
            if(command == nullptr) {
                throw errors::MWSMessageException{"Module \"" + mod->name + "\" does not have command: " + lines[i].source[2].content,global::get_line()};
            }

            std::vector<GeneralTypeToken> args;
            if(command->args.size() != lines[i].source.size()-3) {
                std::string err = "Too many/few arguments for command: " + command->name + "\n\t- Expected: " + std::to_string(command->args.size()) + "\n\t- But got: " + std::to_string(lines[i].source.size()-3);
                throw errors::MWSMessageException{err,global::get_line()};
            }

            for(size_t j = 0; j < command->args.size(); ++j) {
                auto identf = get_type(lines[i].source[j+3],command->args[j]);
                if(!command->args[j].matches(identf)) {
                    std::string err_msg = "Invalid argument:\n\t- Expected: [";
                    for(size_t k = 0; k < command->args[j].carry.size(); ++k) {
                        err_msg += (k == 0 ? "":",") + general_t2token(General_type(command->args[j].carry[k]-1)).content;
                    }
                    err_msg += "]\n\t- But got: " + general_t2token(get_type(lines[i].source[j+3].content)).content + " (" + lines[i].source[j+3].content + ")";
                    throw errors::MWSMessageException(err_msg,global::get_line());
                }
                else {
                    args.push_back(GeneralTypeToken(lines[i].source[j+3],command->args[j]));
                }
            }

            ret = command->run(args);
        }
        else if(identf_line.front() == General_type::STRING || (identf_line.front() == General_type::NAME && is_variable(lines[i].source[0]) && get_variable(lines[i].source[0])->type == Variable::Type::String)) {
            for(size_t j = 1; j < lines[i].source.size(); ++j) {
                identf_line.push_back(get_type(lines[i].source[j]));
            }

            if(identf_line.size() < 3 || lines[i].source[1].content != ".") {
                throw errors::MWSMessageException{"Invalid string-method call!\n\t- Expected: string.method <args>...",global::get_line()};
            }
            auto idf_first = identf_line.front();
            identf_line.erase(identf_line.begin()); // string
            identf_line.erase(identf_line.begin()); // .

            Method<Token>* method = get_string_method(lines[i].source[2].content);
            if(method == nullptr) {
                throw errors::MWSMessageException{"Unknown string method: " + lines[i].source[2].content,global::get_line()};
            }

            std::vector<GeneralTypeToken> args;
            if(method->args.size() != lines[i].source.size()-3) {
                std::string err = "Too many/few arguments for string method: " + method->name + "\n\t- Expected: " + std::to_string(method->args.size()) + "\n\t- But got: " + std::to_string(lines[i].source.size()-3);
                throw errors::MWSMessageException{err,global::get_line()};
            }

            for(size_t j = 0; j < method->args.size(); ++j) {
                auto identf = get_type(lines[i].source[j+3],method->args[j]);
                if(!method->args[j].matches(identf)) {
                    std::string err_msg = "Invalid argument:\n\t- Expected: [";
                    for(size_t k = 0; k < method->args[j].carry.size(); ++k) {
                        err_msg += (k == 0 ? "":",") + general_t2token(General_type(method->args[j].carry[k]-1)).content;
                    }
                    err_msg += "]\n\t- But got: " + general_t2token(get_type(lines[i].source[j+3].content)).content + " (" + lines[i].source[j+3].content + ")";
                    throw errors::MWSMessageException(err_msg,global::get_line());
                }
                else {
                    args.push_back(GeneralTypeToken(lines[i].source[j+3],method->args[j]));
                }
            }

            if(idf_first == General_type::STRING) {
                Token tk = lines[i].source[0];
                ret = method->run(args,&tk);
            }
            else {
                Token tk = get_variable(lines[i].source[0])->storage.string;
                ret = method->run(args,&tk);
                get_variable(lines[i].source[0])->storage.string = tk;
            }
        }
        else if(identf_line.front() == General_type::LIST || (identf_line.front() == General_type::NAME && is_variable(lines[i].source[0]) && get_variable(lines[i].source[0])->type == Variable::Type::List)) {
            for(size_t j = 1; j < lines[i].source.size(); ++j) {
                identf_line.push_back(get_type(lines[i].source[j]));
            }

            if(identf_line.size() < 3 || lines[i].source[1].content != ".") {
                throw errors::MWSMessageException{"Invalid string-method call!\n\t- Expected: string.method <args>...",global::get_line()};
            }
            auto idf_first = identf_line.front();
            identf_line.erase(identf_line.begin()); // string
            identf_line.erase(identf_line.begin()); // .

            Method<List>* method = get_list_method(lines[i].source[2].content);
            if(method == nullptr) {
                throw errors::MWSMessageException{"Unknown list method: " + lines[i].source[2].content,global::get_line()};
            }

            std::vector<GeneralTypeToken> args;
            if(method->args.size() != lines[i].source.size()-3) {
                std::string err = "Too many/few arguments for list method: " + method->name + "\n\t- Expected: " + std::to_string(method->args.size()) + "\n\t- But got: " + std::to_string(lines[i].source.size()-3);
                throw errors::MWSMessageException{err,global::get_line()};
            }

            for(size_t j = 0; j < method->args.size(); ++j) {
                auto identf = get_type(lines[i].source[j+3],method->args[j]);
                if(!method->args[j].matches(identf)) {
                    std::string err_msg = "Invalid argument:\n\t- Expected: [";
                    for(size_t k = 0; k < method->args[j].carry.size(); ++k) {
                        err_msg += (k == 0 ? "":",") + general_t2token(General_type(method->args[j].carry[k]-1)).content;
                    }
                    err_msg += "]\n\t- But got: " + general_t2token(get_type(lines[i].source[j+3].content)).content + " (" + lines[i].source[j+3].content + ")";
                    throw errors::MWSMessageException(err_msg,global::get_line());
                }
                else {
                    args.push_back(GeneralTypeToken(lines[i].source[j+3],method->args[j]));
                }
            }

            if(idf_first == General_type::LIST) {
                List ls = construct_list(lines[i].source[0].content);
                ret = method->run(args,&ls);
            }
            else {
                List ls = get_variable(lines[i].source[0])->storage.list;
                ret = method->run(args,&ls);
                get_variable(lines[i].source[0])->storage.list = ls;
            }
        }
        else if(identf_line.front() == General_type::DICTIONARY || (identf_line.front() == General_type::NAME && is_variable(lines[i].source[0]) && get_variable(lines[i].source[0])->type == Variable::Type::Dictionary)) {
            for(size_t j = 1; j < lines[i].source.size(); ++j) {
                identf_line.push_back(get_type(lines[i].source[j]));
            }

            if(identf_line.size() < 3 || lines[i].source[1].content != ".") {
                throw errors::MWSMessageException{"Invalid dictionary-method call!\n\t- Expected: object.method <args>...",global::get_line()};
            }
            auto idf_first = identf_line.front();
            identf_line.erase(identf_line.begin()); // dictionary
            identf_line.erase(identf_line.begin()); // .

            Method<Dictionary>* method = get_dictionary_method(lines[i].source[2].content);
            if(method == nullptr) {
                throw errors::MWSMessageException{"Unknown dictionary method: " + lines[i].source[2].content,global::get_line()};
            }

            std::vector<GeneralTypeToken> args;
            if(method->args.size() != lines[i].source.size()-3) {
                std::string err = "Too many/few arguments for dictionary method: " + method->name + "\n\t- Expected: " + std::to_string(method->args.size()) + "\n\t- But got: " + std::to_string(lines[i].source.size()-3);
                throw errors::MWSMessageException{err,global::get_line()};
            }

            for(size_t j = 0; j < method->args.size(); ++j) {
                auto identf = get_type(lines[i].source[j+3],method->args[j]);
                if(!method->args[j].matches(identf)) {
                    std::string err_msg = "Invalid argument:\n\t- Expected: [";
                    for(size_t k = 0; k < method->args[j].carry.size(); ++k) {
                        err_msg += (k == 0 ? "":",") + general_t2token(General_type(method->args[j].carry[k]-1)).content;
                    }
                    err_msg += "]\n\t- But got: " + general_t2token(get_type(lines[i].source[j+3].content)).content + " (" + lines[i].source[j+3].content + ")";
                    throw errors::MWSMessageException(err_msg,global::get_line());
                }
                else {
                    args.push_back(GeneralTypeToken(lines[i].source[j+3],method->args[j]));
                }
            }

            if(idf_first == General_type::DICTIONARY) {
                Dictionary dic = dic_from_token(lines[i].source[0]);
                ret = method->run(args,&dic);
            }
            else {
                Dictionary dic = get_variable(lines[i].source[0])->storage.dict;
                ret = method->run(args,&dic);
                get_variable(lines[i].source[0])->storage.dict = dic;
            }
        }
        else if(identf_line.front() == General_type::STRUCT) {
            for(size_t j = 1; j < lines[i].source.size(); ++j) {
                identf_line.push_back(get_type(lines[i].source[j]));
            }

            if(lines[i].source.size() != 3 || get_type(lines[i].source[1]) != General_type::NAME ||
                 get_type(lines[i].source[2],car_ArgumentList) != General_type::ARGUMENTLIST) {
                throw errors::MWSMessageException{"Invalid pattern for a struct initialisation!\n\t- Pattern: <struct> <name> (<args>)",global::get_line()};
            }
            Token struct_name = lines[i].source[0];
            Token instance_name = lines[i].source[1];

            argument_list args = tools::parse_argument_list(lines[i].source[2]);
            std::vector<Variable> call_args;
            for(auto i : args)
                call_args.push_back(i.to_variable());

            int scope_idx = get_new_scope();
            Object obj;
            Object* struc = get_struct(struct_name);
            obj.methods = struc->methods;
            obj.members = struc->members;
            obj.parent_scope = struc->parent_scope;
            obj.structs = struc->structs;
            obj.on_deconstruct = struc->on_deconstruct;

            for(auto& i : obj.methods) {
                int sscope = i.second.scope_idx;
                i.second.scope_idx = get_new_scope();
                scopes[i.second.scope_idx] = scopes[sscope];
                scopes[i.second.scope_idx].parent = obj.parent_scope;
                scopes[i.second.scope_idx].index = i.second.scope_idx;
            }
            set_variable(instance_name,obj);
            if(has_method(obj,struct_name)) {
                run_method(obj,struct_name,call_args);
            }
            set_variable(instance_name,obj);
        }
        else if(identf_line.front() == General_type::OBJECT) {
            for(size_t j = 1; j < lines[i].source.size(); ++j) {
                identf_line.push_back(get_type(lines[i].source[j]));
            }

            if(lines[i].source.size()-1 < 3 || lines[i].source[1].content != "." || lines[i].source[1].in_quotes) {
                throw errors::MWSMessageException{"Invalid object-method call!\n\t- Expected: object.method <args>...",global::get_line()};
            }
            auto cpy = lines[i].source;
            cpy.erase(cpy.begin()); // object
            cpy.erase(cpy.begin()); // .

            Object* obj = get_object(lines[i].source[0].content);
            Token method_name = lines[i].source[2];
            Function* method = get_method(obj,method_name);
            if(method == nullptr) {
                throw errors::MWSMessageException{"Unknown object method: " + method_name.content,global::get_line()};
            }

            std::vector<Variable> args; // TODO: add boring error handling
            std::vector<GeneralTypeToken> arglist = tools::parse_argument_list(lines[i].source[3]);
            if(lines[i].source.size() != 4) {
                throw errors::MWSMessageException{"Invalid object-method call!\n\t- Expected: object.method <args>...",global::get_line()};
            }
            if(arglist.size() != method->params.size()) {
                std::string err = "Too many/few arguments for object method: " + method_name.content + "\n\t- Expected: " + std::to_string(method->params.size()) + "\n\t- But got: " + std::to_string(lines[i].source.size()-3);
                throw errors::MWSMessageException{err,global::get_line()};
            }

            for(size_t j = 0; j < method->params.size(); ++j) {
                Variable garg = arglist[j].to_variable(); 
                auto identf = garg.type;
                if(!method->params[j].matches(garg)) {
                    std::string err_msg = "Invalid argument:\n\t- Expected: " + var_t2token(method->params[j].type).content + "\n\t- But got: " + general_t2token(get_type(lines[i].source[j+3].content)).content + " (" + lines[i].source[j+3].content + ")";
                    throw errors::MWSMessageException(err_msg,global::get_line());
                }
                else {
                    args.push_back(garg);
                }
            }

            ret = run_method(*obj,method_name,args);
        }
        else if(identf_line.front() == General_type::EXPRESSION) {
            for(size_t j = 1; j < lines[i].source.size(); ++j) {
                identf_line.push_back(get_type(lines[i].source[j]));
            }
            bool shadow_return = false;
            if(identf_line.size() > 2) {
                std::string err = "Invalid start of line!\n\t- Expected: [Command,Function,Module,String,List,Dictionary,Object,Struct]\n\t- But got: " + general_t2token(identf_line.front()).content + " (" + lines[i].source[0].content + ")\n"; 
                throw errors::MWSMessageException{err,global::get_line()};
            }
            if(identf_line.size() == 2) {
                if(identf_line[1] != General_type::OPERATOR) {
                    std::string err = "Unexpected token after expression:\n\t- Expected: \"!\"\n\t- But got: " + general_t2token(identf_line[1]).content;
                    throw errors::MWSMessageException{err,global::get_line()};
                }
                if(lines[i].source[1].content != "!") {
                    std::string err = "Unexpected token after expression:\n\t- Expected: \"!\"\n\t- But got: " + lines[i].source[2].content;
                    throw errors::MWSMessageException{err,global::get_line()};
                }
                shadow_return = true;
            }
            if(shadow_return) {
                parse_expression(lines[i].source[0]);
            }
            else {
                ret = parse_expression(lines[i].source[0]);
            }
        }
        else {
            std::string err = "Invalid start of line!\n\t- Expected: [Command,Function,Module,String,List,Dictionary,Object,Struct]\n\t- But got: " + general_t2token(identf_line.front()).content + " (" + lines[i].source[0].content + ")\n"; 
            throw errors::MWSMessageException{err,global::get_line()};
        }

        global::pop_trace();

        if(ret.type == General_type::OBJECT) {
            Object* obj = get_object(ret.source);
            if(obj != nullptr) {
                ret.saveobj = *obj;
                ret.use_save_obj = true;
            }
        }

        if(global::runner_should_return != 0) {
            if(!pass_return_down) {
                --global::runner_should_return;
            }
            if(from != "") {
                global::include_path.pop();
            }
            if(!same_scope) {
                pop_scope(save_scope);
            }
            global::line_count.pop();
            if(same_scope) {
                current_scope()->current_line = ln;
            }
            return ret;
        }
        else if(global::runner_should_exit != 0) {
            if(from != "") {
                global::include_path.pop();
            }
            if(!same_scope) {
                pop_scope(save_scope);
            }
            global::line_count.pop();
            if(same_scope) {
                current_scope()->current_line = ln;
            }
            return GeneralTypeToken();
        }
        else if(global::break_loop != 0 || global::continue_loop != 0) {
            if(from != "") {
                global::include_path.pop();
            }
            if(!same_scope) {
                pop_scope(save_scope);
            }
            global::line_count.pop();
            if(same_scope) {
                current_scope()->current_line = ln;
            }
            return GeneralTypeToken();
        }
        else if(global::in_compound != 0 && i+1 == lines.size()) {
            if(from != "") {
                global::include_path.pop();
            }
            if(!same_scope) {
                pop_scope(save_scope);
            }
            global::line_count.pop();
            if(same_scope) {
                current_scope()->current_line = ln;
            }
            return ret;
        }
        else if(ret.type != General_type::VOID && ret.type != General_type::UNKNOWN) {
            std::cout << ret.to_string() << "\n";
        }
    }

    if(from != "") {
        global::include_path.pop();
    }
    if(!same_scope) {
        pop_scope(save_scope);
    }
    global::line_count.pop();
    if(same_scope) {
        current_scope()->current_line = ln;
    }
    return general_null;
}

GeneralTypeToken MeowScript::run_text(std::string text, bool new_scope, bool save_scope, int load_idx, std::map<std::string,Variable> external_vars, fs::path from, bool pass_return_down, bool same_scope) {
    auto lines = lex_text(text);
    return run_lexed(lines,new_scope,save_scope,load_idx,external_vars,from,pass_return_down,same_scope);
}