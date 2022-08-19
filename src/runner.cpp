#include "../inc/runner.hpp"
#include "../inc/tools.hpp"
#include "../inc/modules.hpp"

MEOWSCRIPT_SOURCE_FILE

GeneralTypeToken MeowScript::run_file(std::string file, bool new_scope, bool save_scope, int load_idx, std::map<std::string,Variable> external_vars, fs::path from, bool pass_return_down) {
    if(from != "") {
        return run_text(from.remove_filename().string() + MEOWSCRIPT_DIR_SL + read(file),true,save_scope,load_idx,external_vars,from,pass_return_down);
    }
    else if(!global::include_path.empty()) {
        return run_text(read(global::include_path.top().remove_filename().string() + MEOWSCRIPT_DIR_SL + file),true,save_scope,load_idx,external_vars,from,pass_return_down);
    }
    else {
        return run_text(read(file),true,save_scope,load_idx,external_vars,from,pass_return_down);
    }
}

GeneralTypeToken MeowScript::run_lexed(lexed_tokens lines, bool new_scope, bool save_scope, int load_idx, std::map<std::string,Variable> external_vars, fs::path from, bool pass_return_down) {
    if(from != "") {
        global::include_path.push(from);
    }
    if(load_idx < 0) {
        if(!new_scope) {
            save_scope = false; // potential "memory leak" if else!
        }
        MeowScript::new_scope(new_scope ? -2:-1,external_vars);
    }
    else {
        load_scope(load_idx,external_vars);
    }
    global::line_count.push(0);

    for(size_t i = 0; i < lines.size(); ++i) {
        GeneralTypeToken ret;
        global::line_count.top() = lines[i].line_count;
        if(lines[i].source.empty()) {
            continue;
        }

        std::vector<General_type> identf_line;
        identf_line.push_back(get_type(lines[i].source[0]));

        std::string name = lines[i].source[0];

        if(identf_line.front() == General_type::COMMAND) {
            Command* command = get_command(name);

            std::vector<GeneralTypeToken> args;
            identf_line.erase(identf_line.begin());
            if(command->args.size() != lines[i].source.size()-1) {
                std::string err = "Too many/few arguments for command: " + command->name + "\n\t- Expected: " + std::to_string(command->args.size()) + "\n\t- But got: " + std::to_string(lines[i].source.size()-1);
                throw errors::MWSMessageException{err,global::get_line()};
            }

            for(size_t j = 0; j < command->args.size(); ++j) {
                auto identf = get_type(lines[i].source[j+1],command->args[j]);
                if(!command->args[j].matches(identf)) {
                    std::string err_msg = "Invalid argument:\n\t- Expected: [";
                    for(size_t k = 0; k < command->args[j].carry.size(); ++k) {
                        err_msg += (k == 0 ? "":",") + general_t2token(General_type(command->args[j].carry[k]-1)).content;
                    }
                    err_msg += "]\n\t- But got: " + general_t2token(identf).content + " (" + lines[i].source[j+1].content + ")";
                    throw errors::MWSMessageException(err_msg,global::get_line());
                }
                else {
                    args.push_back(GeneralTypeToken(lines[i].source[j+1]));
                }
            }

            ret = command->run(args);
        }
        else if(identf_line.front() == General_type::FUNCTION) {
            for(size_t j = 1; j < lines[i].source.size(); ++j) {
                identf_line.push_back(get_type(lines[i].source[j]));
            }
            if(identf_line.size() != 2 || identf_line[1] != General_type::ARGUMENTLIST) {
                std::string err = "Unexpected token after function call:\n\t- Expected: ARGUMENTLIST\n\t- But got: " + (identf_line.size() != 1 ? general_t2token(identf_line[1]).content : "VOID");
                throw errors::MWSMessageException{err,global::get_line()};
            }

            Function* fun = get_function(name);

            argument_list alist = tools::parse_argument_list(lines[i].source[1]);

            if(alist.size() != fun->args.size()) {
                std::string err = "Too many/few arguments for function: " + name + "\n\t- Expected: " + std::to_string(fun->args.size()) + "\n\t- But got: " + std::to_string(alist.size());
                throw errors::MWSMessageException{err,global::get_line()};
            }

            for(size_t j = 0; j < alist.size(); ++j) {
                alist[j] = tools::check4placeholder(alist[j]);
            }

            std::vector<Variable> args;
            for(size_t j = 0; j < fun->args.size(); ++j) {
                try {
                    if(fun->args[j] != Variable::Type::UNKNOWN && fun->args[j] != general_t2var_t(alist[j].type)) {
                        std::string err_msg = "Invalid argument:\n\t- Expected: " + var_t2token(fun->args[j]).content + "\n\t- But got: " + var_t2token(alist[j].to_variable().type).content;
                        throw errors::MWSMessageException(err_msg,global::get_line());
                    }
                    else {
                        ++global::in_argument_list;
                        args.push_back(alist[j].to_variable());
                        --global::in_argument_list;
                    }
                }
                catch(errors::MWSMessageException& err) {
                    std::string err_msg = "Can't convert GeneralType " + general_t2token(alist[j].type).content + " to VariableType " + var_t2token(fun->args[j]).content + " as function parameter for function: " + name;
                    throw errors::MWSMessageException{err_msg,global::get_line()};
                }
            }

            ret = GeneralTypeToken(fun->run(args));
            if(fun->return_type == Variable::Type::VOID && ret.type != General_type::UNKNOWN && ret.type != General_type::VOID) {
                throw errors::MWSMessageException{"Invalid return type!\n\t- Expected: Void\n\t- But got: " + general_t2token(ret.type).content,global::get_line()};
            }
            else if(fun->return_type != Variable::Type::VOID && fun->return_type != Variable::Type::ANY && var_t2general_t(fun->return_type) != ret.type) {
                throw errors::MWSMessageException{"Invalid return type!\n\t- Expected: " + var_t2token(fun->return_type).content + "\n\t- But got: " + general_t2token(ret.type).content,global::get_line()};
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
                    args.push_back(GeneralTypeToken(lines[i].source[j+3]));
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
                    args.push_back(GeneralTypeToken(lines[i].source[j+3]));
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
                    args.push_back(GeneralTypeToken(lines[i].source[j+3]));
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
        else {
            std::string err = "Invalid start of line!\n\t- Expected: [Command,Function,Module,String,List]\n\t- But got: " + general_t2token(identf_line.front()).content + " (" + lines[i].source[0].content + ")\n"; 
            throw errors::MWSMessageException{err,global::get_line()};
        }

        if(global::runner_should_return != 0) {
            if(!pass_return_down) {
                --global::runner_should_return;
            }
            if(from != "") {
                global::include_path.pop();
            }
            pop_scope(save_scope);
            global::line_count.pop();
            return ret;
        }
        else if(global::runner_should_exit != 0) {
            if(from != "") {
                global::include_path.pop();
            }
            pop_scope(save_scope);
            global::line_count.pop();
            return GeneralTypeToken();
        }
        else if(global::break_loop != 0 || global::continue_loop != 0) {
            if(from != "") {
                global::include_path.pop();
            }
            pop_scope(save_scope);
            global::line_count.pop();
            return GeneralTypeToken();
        }
        else if(global::in_compound != 0 && i+1 == lines.size()) {
            if(from != "") {
                global::include_path.pop();
            }
            pop_scope(save_scope);
            global::line_count.pop();
            return ret;
        }
        else if(ret.type != General_type::VOID && ret.type != General_type::UNKNOWN) {
            std::cout << ret.to_string() << "\n";
        }
    }

    if(from != "") {
        global::include_path.pop();
    }
    pop_scope(save_scope);
    global::line_count.pop();
    return GeneralTypeToken();
}

GeneralTypeToken MeowScript::run_text(std::string text, bool new_scope, bool save_scope, int load_idx, std::map<std::string,Variable> external_vars, fs::path from, bool pass_return_down) {
    auto lines = lex_text(text);
    return run_lexed(lines,new_scope,save_scope,load_idx,external_vars,from,pass_return_down);
}