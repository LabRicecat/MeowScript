#include "../inc/commands.hpp"
#include "../inc/runner.hpp"
#include "../inc/modules.hpp"
#include "../inc/variables.hpp"
#include "../inc/global.hpp"
#include "../inc/tools.hpp"
#include "../inc/expressions.hpp"

MEOWSCRIPT_SOURCE_FILE

bool MeowScript::CommandArgReqirement::has_carry(int in) const {
    for(auto i : carry) {
        if(i == in) {
            return true;
        }
    }
    return false;
}

bool MeowScript::CommandArgReqirement::matches(Token tk) {
    General_type t = get_type(tk);
    return matches(t);
}

bool MeowScript::CommandArgReqirement::matches(CommandArgReqirement car) {
    for(auto i : carry) {
        for(auto j : car.carry) {
            if(i == j || i == 0 || j == 0) {
                return true;
            }
        }
    }
    return false;
}
/*
bool MeowScript::CommandArgReqirement::matches(General_type type) {
    for(auto i : carry) {
        if(i == 0 || i == (static_cast<int>(type)+1)) {
            return true;
        }
    }
    return false;
}*/

bool MeowScript::is_command(std::string name) {
    for(auto i : *get_command_list()) {
        if(i.name == name) {
            return true;
        }
    }
    return false;
}

Command* MeowScript::get_command(std::string name) {
    for(auto& i : *get_command_list()) {
        if(i.name == name) {
            return &i;
        }
    }
    return nullptr;
}

Command* MeowScript::get_command_overload(std::string name,std::vector<Token> tokens) {
    for(auto& i : *get_command_list()) {
        if(i.name == name) {
            if(tokens.size() != i.args.size() && !(i.args.size() != 0 && i.args.back().matches(car_Ongoing) && i.args.back() != car_Any)) {
                continue;
            }
            bool failed = false;
            for(size_t j = 0; j < i.args.size(); ++j) {
                auto identf = get_type(tokens[j],i.args[j]);
                if(i.args[j] == car_Ongoing) {
                    return &i;
                }
                if(!i.args[j].matches(identf)) {
                    failed = true;
                    break;
                }
            }
            if(failed) { continue; }
            return &i;
        }
    }
    return nullptr;
}

static std::vector<Command> commandlist = {
    {"new",
        {
            car_Name,
            car_Number | car_String | car_List | car_Dictionary | car_PlaceHolderAble,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_CAN_BE_IN_STRUCT()
        new_variable(args[0].source.content,tools::check4placeholder(args[1]).to_variable());
        return general_null;
    }},
    {"new",
        {
            car_Name,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_CAN_BE_IN_STRUCT()
        new_variable(args[0].source.content,0);
        return general_null;
    }},
    {"const",
        {
            car_Name,
            car_Number | car_String | car_List | car_Dictionary | car_PlaceHolderAble,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_CAN_BE_IN_STRUCT()
        Variable v = tools::check4placeholder(args[1]).to_variable();
        v.constant = true;
        new_variable(args[0].source.content,tools::check4placeholder(args[1]).to_variable());
        return general_null;
    }},
    {"set",
        {
            car_Name,
            car_Number | car_String | car_List | car_Dictionary | car_PlaceHolderAble,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        set_variable(args[0].source.content,tools::check4placeholder(args[1]).to_variable());
        return general_null;
    }},
    {"look",
        {
            car_Name
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        Variable* var = get_variable(args[0].source.content);
        if(var == nullptr) {
            throw errors::MWSMessageException{"Unknwon variable: " + args[0].source.content,global::get_line()};
        }
        return GeneralTypeToken(*var);
    }},
    {"return",
        {
            car_Any
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        auto ret = tools::check4placeholder(args[0]);
        ++global::runner_should_return;
        return ret;
    }},
    {"return",
        {
            
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        ++global::runner_should_return;
        return general_null;
    }},
    {"func",
        {
            car_Name | car_Function,
            car_ParameterList, 
            car_Operator, // ->
            car_Name | car_Struct, // ReturnValue
            car_Compound,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_CAN_BE_IN_STRUCT()
        Function fun;
        std::string ret_ty = args[3].to_string();

        if(args[2].to_string() != "->") {
            throw errors::MWSMessageException{"Expected \"->\" to declarate a return value, but got: " + args[2].to_string(),global::get_line()};
        }
        if(!is_struct(ret_ty) && !is_valid_var_t(ret_ty) && ret_ty != "Any" && ret_ty != "Void") {
            throw errors::MWSMessageException{ret_ty + " is not a valid return type!",global::get_line()};
        }

        if(ret_ty == "Any") {
            fun.return_type = Variable::Type::ANY;
        }
        else if(ret_ty == "Void") {
            fun.return_type = Variable::Type::VOID;
        }
        else if(args[3].type == General_type::STRUCT) {
            fun.return_type.type = Variable::Type::Object;
            fun.return_type.struct_name = args[3].source.content;
        }
        else if(ret_ty == "Object") {
            fun.return_type.type = Variable::Type::Object;
            fun.return_type.struct_name = "";
        }
        else {
            fun.return_type = token2var_t(args[3].to_string());
        }

        args[4].source.content.erase(args[4].source.content.begin());
        args[4].source.content.erase(args[4].source.content.begin()+args[4].source.content.size()-1);
        
        fun.body = lex_text(args[4].source.content);

        fun.params = tools::parse_function_params(args[1].source);

        fun.file = global::include_path.top();
        if(!add_function(args[0].source.content,fun)) {
            throw errors::MWSMessageException{"Can't redefine function: " + args[0].source.content,global::get_line()};
        }
        return general_null;
    }},
    {"func",
        {
            car_Name | car_Function,
            car_ParameterList, 
            car_Compound | car_Expression | car_Name | car_String | car_Number
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_CAN_BE_IN_STRUCT()
        Command* real_func = get_command("func");
        GeneralTypeToken any;
        any.type = General_type::NAME;
        any.source.content = "Any";
        args.insert(args.begin() + 2,any);
        GeneralTypeToken op;
        op.type = General_type::OPERATOR;
        op.source.content = "->";
        args.insert(args.begin() + 2,op);
        
        GeneralTypeToken comp;
        comp.type = General_type::COMPOUND;
        comp.source = "{ return ( " + args[2].to_string() + " ) }";
        args.push_back(comp);

        real_func->run(args);
        return general_null;
    }},
    {"func",
        {
            car_Name | car_Function,
            car_ParameterList, 
            car_Operator, // =>
            car_Compound | car_Expression | car_Name | car_String | car_Number | car_Ongoing
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_CAN_BE_IN_STRUCT()
        Command* real_func = get_command("func");

        if(args[2].to_string() != "=>") {
            throw errors::MWSMessageException{"Expected \"=>\" to declarate a return value for this function, but got: " + args[2].to_string(),global::get_line()};
        }
        args.erase(args.begin()+2);

        std::string ret;
        for(size_t i = 2; i < args.size(); ++i) {
            ret += args[i].to_string() + " ";
        }
        GeneralTypeToken comp;
        comp.type = General_type::COMPOUND;
        if(args.size() > 3) {
            comp.source = "{ return ( " + ret + " ) }";
        }
        else {
            comp.source = "{ return " + ret + " }";
        }
        

        while(args.size() > 2) {
            args.erase(args.end()-1);
        }

        GeneralTypeToken op;
        op.type = General_type::OPERATOR;
        op.source.content = "->";
        args.push_back(op);
        GeneralTypeToken any;
        any.type = General_type::NAME;
        any.source.content = "Any";
        args.push_back(any);

        args.push_back(comp);

        real_func->run(args);
        return general_null;
    }},

    {"print",
        {
            car_String | car_Number | car_List | car_Expression | car_PlaceHolderAble
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        GeneralTypeToken gtt = tools::check4placeholder(args[0]);
        if(gtt.type == General_type::STRING) {
            std::cout << gtt.source.content << "\n";
        }
        else if(gtt.type == General_type::NAME) {
            throw errors::MWSMessageException{"Unknown symbol: " + gtt.source.content,global::get_line()};
        }
        else {
            std::cout << gtt.to_string() << "\n";
        }
        return general_null;
    }},
    {"print",
        {
            car_Ongoing
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        std::string newline = "\n";

        if(args.size() != 0 && args.back().type == General_type::EXPRESSION) {
            auto cpy = args.back().source.content;
            cpy.erase(cpy.begin());
            cpy.erase(cpy.end()-1);

            auto lexed = lex_text(cpy);
            std::vector<Token> line;
            for(auto i : lexed) {
                for(auto j : i.source) {
                    line.push_back(j);
                }
            }
            if(line.size() >= 3 && line.front().content == "end") {
                new_scope();
                new_variable("end",0);
                std::string expr;
                for(auto i : line) {
                    if(i.in_quotes) {
                        expr += "\"" + i.content + "\"";
                    }
                    else {
                        expr += i.content;
                    }
                    expr += " ";
                }
                parse_expression("( " + expr + " )");
                Variable end = *get_variable("end");
                if(end.type == Variable::Type::String) {
                    newline = end.storage.string.content;
                }
                else {
                    newline = end.to_string();
                }
                pop_scope();
                args.pop_back();
            }
        }

        for(auto i : args) {
            GeneralTypeToken gtt = tools::check4placeholder(i);
            if(gtt.type == General_type::STRING) {
                std::cout << gtt.source.content;
            }
            else if(gtt.type == General_type::NAME) {
                throw errors::MWSMessageException{"Unknown symbol: " + gtt.source.content,global::get_line()};
            }
            else {
                std::cout << gtt.to_string();
            }
        }
        std::cout << newline;
        return general_null;
    }},

    {"if",
        {
            car_Expression,
            car_Compound,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        auto res = parse_expression("(" + args[0].to_string() + ")");
        if(res.type != Variable::Type::Number) {
            throw errors::MWSMessageException{std::string("Expression \"" + args[0].to_string() + "\" did not return VariableType \"Number\" for if-statement!"),global::get_line()};
        }

        args[1].source.content.erase(args[1].source.content.begin());
        args[1].source.content.erase(args[1].source.content.begin()+args[1].source.content.size()-1);

        current_scope()->last_if_result = (res.storage.number == 1);
        if(res.storage.number == 1) {
            auto ret = run_text(args[1].source.content,false,false,-1,{},"",true);
            if(global::runner_should_return != 0) {
                return ret;
            }
        }
        return general_null;
    }},
    {"elif",
        {
            car_Expression,
            car_Compound,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        auto res = parse_expression(args[0].to_string());
        if(res.type != Variable::Type::Number) {
            throw errors::MWSMessageException{std::string("Expression \"" + args[0].to_string() + "\" did not return VariableType \"Number\" for if-statement!"),global::get_line()};
        }

        args[1].source.content.erase(args[1].source.content.begin());
        args[1].source.content.erase(args[1].source.content.begin()+args[1].source.content.size()-1);

        if(res.storage.number == 1 && !current_scope()->last_if_result) {
            auto ret = run_text(args[1].source.content,false,false);
            current_scope()->last_if_result = true;
            if(ret.type != General_type::VOID && ret.type != General_type::UNKNOWN) {
                ++global::runner_should_return;
                current_scope()->last_if_result = (res.storage.number == 1);
                return ret;
            }
        }
        return general_null;
    }},
    {"else",
        {
            car_Compound,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        args[0].source.content.erase(args[0].source.content.begin());
        args[0].source.content.erase(args[0].source.content.begin()+args[0].source.content.size()-1);

        if(!current_scope()->last_if_result) {
            auto ret = run_text(args[0].source.content,false,false,-1,{},"",true);
            if(global::runner_should_return != 0) {
                current_scope()->last_if_result = true;
                return ret;
            }
        }
        current_scope()->last_if_result = true;
        return general_null;
    }},
    {"using",
        {
            car_Name | car_Module
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        if(args[0].type == General_type::MODULE && is_loaded_module(args[0].to_string())) {
            get_module(args[0].to_string())->enabled = true;
        }
        else if(is_loadable_module(args[0].to_string())) {
            load_module(args[0].to_string() + MEOWSCRIPT_SHARED_OBJECT_EXT);
        }
        else {
            throw errors::MWSMessageException{"Could not load module: " + args[0].to_string() + " -> not installed.",global::get_line()};
        }
        return general_null;
    }},
    {"for",
        {
            car_Name,
            car_Any,
            car_Number | car_String | car_List | car_Dictionary | car_PlaceHolderAble,
            car_Compound
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        if(args[1].to_string() != "in") {
            throw errors::MWSMessageException{"Expected \"in\" for loop but got: " + args[1].to_string(),global::get_line()};
        }
        GeneralTypeToken to_iterate = tools::check4placeholder(args[2]);
        std::string var_name = args[0].to_string();
        std::string comp = args[3].to_string();
        comp.erase(comp.begin());
        comp.erase(comp.begin()+comp.size()-1);

        if(to_iterate.type == General_type::LIST) {
            for(auto i : to_iterate.to_variable().storage.list.elements) {
                ++global::in_loop;
                auto ret = run_text(comp,false,false,-1,{{var_name,i}},"",true);
                --global::in_loop;
                if(global::runner_should_return != 0) {
                    return ret;
                }
                if(global::break_loop != 0) {
                    --global::break_loop;
                    return general_null;
                }
                if(global::continue_loop != 0) {
                    --global::continue_loop;
                }
            }
        }
        else if(to_iterate.type == General_type::NUMBER) {
            for(size_t i = 0; i < to_iterate.to_variable().storage.number; ++i) {
                ++global::in_loop;
                auto ret = run_text(comp,false,false,-1,{{var_name,i}},"",true);
                --global::in_loop;
                if(global::runner_should_return != 0) {
                    return ret;
                }
                if(global::break_loop != 0) {
                    --global::break_loop;
                    return general_null;
                }
                if(global::continue_loop != 0) {
                    --global::continue_loop;
                }
            }
        }
        else if(to_iterate.type == General_type::STRING) {
            for(size_t i = 0; i < to_iterate.to_variable().storage.string.content.size(); ++i) {
                ++global::in_loop;
                auto ret = run_text(comp,false,false,-1,{{var_name,Token(std::string(1,to_iterate.to_variable().storage.string.content[i]))}},"",true);
                --global::in_loop;
                if(global::runner_should_return != 0) {
                    return ret;
                }
                if(global::break_loop != 0) {
                    --global::break_loop;
                    return general_null;
                }
                if(global::continue_loop != 0) {
                    --global::continue_loop;
                }
            }
        }
        else {
            throw errors::MWSMessageException{"Unexpected element to loop over!\n\t- Expected: [NUMBER,STRING,LIST]\n\t- But got: " + general_t2token(to_iterate.type).content,global::get_line()};
        }
        return general_null;
    }},
    {"while",
        {
            car_Expression,
            car_Compound
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        GeneralTypeToken gtt = args[0];
        std::string comp = args[1].to_string();
        comp.erase(comp.begin());
        comp.erase(comp.begin()+comp.size()-1);

        GeneralTypeToken check = parse_expression(gtt.to_string());

        while(check.type == General_type::NUMBER && check.to_variable().storage.number == 1) {
            ++global::in_loop;
            auto ret = run_text(comp,false,false,-1,{},"",true);
            --global::in_loop;
            if(global::runner_should_return != 0) {
                return ret;
            }
            if(global::break_loop != 0) {
                --global::break_loop;
                return general_null;
            }
            if(global::continue_loop != 0) {
                --global::continue_loop;
            }
            check = parse_expression(gtt.to_string());
        }
        return general_null;
    }},
    {"break",
        {},
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        ++global::break_loop;
        return general_null;
    }},
    {"continue",
        {},
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        ++global::continue_loop;
        return general_null;
    }},
    {"typeof",
        {
            car_Any,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        GeneralTypeToken gtt;
        gtt.source = general_t2token(tools::check4placeholder(args[0]).type).content;
        gtt.source.in_quotes = true;
        gtt.type = General_type::STRING;
        return gtt;
    }},
    {"input",
        {
            car_ArgumentList
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() > 1) {
            throw errors::MWSMessageException{"Too many/few arguments for command: input\n\t- Expected: <=1\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
        }
        if(alist.size() == 1) {
            auto output = tools::check4placeholder(alist[0]);
            if(output.type != General_type::STRING) {
                throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String\n\t- But got: " + general_t2token(output.type).content,global::get_line()};
            }
            std::cout << output.source.content;
        }
        std::cout.flush();
        std::string inp;
        std::getline(std::cin,inp);
        
        GeneralTypeToken gtt;
        gtt.source = inp;
        gtt.source.in_quotes = true;
        gtt.type = General_type::STRING;
        return gtt;
    }},
        // casts
    {"string",
        {
            car_ArgumentList
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for cast: string()",global::get_line()};
        }
        alist[0] = tools::check4placeholder(alist[0]);
        switch(alist[0].type) {
            case General_type::STRING:
                return alist[0];
            case General_type::NUMBER:
                return GeneralTypeToken(tools::remove_uness_decs(std::to_string(alist[0].to_variable().storage.number),false).content);
            case General_type::LIST:
                {
                    GeneralTypeToken ret;
                    ret.type = General_type::STRING;
                    ret.source = alist[0].to_variable().storage.list.to_string();
                    ret.source.in_quotes = true;
                    return ret;
                }
            default:
                throw errors::MWSMessageException{"Invalid argument for case: string()\n\t- Expected: [String,Number,List]\n\t- But got: " + general_t2token(alist[0].type).content,global::get_line()};
        }
        return general_null;
    }},
    {"number",
        {
            car_ArgumentList
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for cast: number()",global::get_line()};
        }
        alist[0] = tools::check4placeholder(alist[0]);
        switch(alist[0].type) {
            case General_type::STRING:
                if(!all_numbers(alist[0].source.content)) {
                    return general_null;
                }
                try {
                    return std::stod(alist[0].source.content);
                }
                catch(...) {
                    return general_null;
                }
            case General_type::NUMBER:
                return alist[0];
            default:
                return general_null;
        }
        return general_null;
    }},
    {"list",
        {
            car_ArgumentList
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for cast: list()",global::get_line()};
        }
        alist[0] = tools::check4placeholder(alist[0]);
        switch(alist[0].type) {
            case General_type::STRING:
                {
                    List ret;
                    std::string iter = alist[0].source.content;
                    for(auto i : iter) {
                        Token tk;
                        tk.content = i;
                        tk.in_quotes = true;
                        ret.elements.push_back(Variable(tk));
                    }
                    return ret;
                }
            case General_type::LIST:
                return alist[0].to_variable().storage.list;
            case General_type::NUMBER:
                {
                    List ret;
                    for(size_t i = 0; i < alist[0].to_variable().storage.number; ++i) {
                        ret.elements.push_back(i);
                    }
                }
            default:
                return general_null;
        }
        return general_null;
    }},
    {"str2var",
        {
            car_ArgumentList
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for cast: str2var()",global::get_line()};
        }
        alist[0] = tools::check4placeholder(alist[0]);
        if(alist[0].type != General_type::STRING) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String\n\t- But got: " + general_t2token(alist[0].type).content,global::get_line()};
        }
        try {
            return make_variable(alist[0].source.content);
        }
        catch(errors::MWSMessageException& err) {
            throw errors::MWSMessageException{"Invalid cast!\n\t- Can't cast string: \"" + alist[0].to_string() + "\" to a variable!",global::get_line()};
        }

        return general_null;
    }},

    {"import",
        {
            car_String,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        fs::path pth = global::include_path.top();
        pth = pth.remove_filename().string() + args[0].source.content;
        fs::path pth2;

        if(!fs::exists(pth)) {
            pth2 = pth.string() + ".mws";
            if(!fs::exists(pth2)) {
                throw errors::MWSMessageException{"Trying to import unknown file: \"" + pth.string() + "\"",global::get_line()};
            }
            pth = pth2;
        }

        if(!global::is_imported(pth)) {
            global::imported_files.push_back(pth.string());
            return run_file(pth.string(),true,false,-1,{},pth,false,true);   
        }
        return general_null;
    }},

    {"event",
        {
            car_Keyword,
            car_Name,
            car_ArgumentList
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        if(is_event(args[1].source.content)) {
            throw errors::MWSMessageException{"Can't define event: \"" + args[1].source.content + "\"",global::get_line()};
        }

        Event event;
        event.from_file = global::include_path.top().string();
        if(args[0].to_string() == "public") {
            event.visibility = Event::Visibility::PUBLIC;
        }
        else if(args[0].to_string() == "private") {
            event.visibility = Event::Visibility::PRIVATE;
        }
        else if(args[0].to_string() == "listen_only") {
            event.visibility = Event::Visibility::LISTEN_ONLY;
        }
        else if(args[0].to_string() == "occur_only") {
            event.visibility = Event::Visibility::OCCUR_ONLY;
        }
        else {
            throw errors::MWSMessageException{"Unknown visibility: \"" + args[0].source.content + "\".\nKnown are: [public,private,call_only,occur_only]",global::get_line()};
        }

        event.params = tools::parse_function_params(args[2].source);

        global::events[args[1].source.content] = event;
        return general_null;
    }},
    {"occur",
        {
            car_Event,
            car_ArgumentList
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        if(!is_event(args[0].source.content)) {
            throw errors::MWSMessageException{"No such event to occur: \"" + args[0].source.content + "\"",global::get_line()};
        }
        Event eve = global::events[args[0].source.content];
        if((eve.visibility != Event::Visibility::PUBLIC && eve.visibility != Event::Visibility::OCCUR_ONLY) && eve.from_file != global::include_path.top()) {
            throw errors::MWSMessageException{"Can't occur inaccessable event: \"" + args[0].source.content + "\"",global::get_line()};
        }

        argument_list alist = tools::parse_argument_list(args[1].source);

        if(alist.size() != eve.params.size()) {
            std::string err = "Too many/few arguments for event: " + args[0].source.content + "\n\t- Expected: " + std::to_string(eve.params.size()) + "\n\t- But got: " + std::to_string(alist.size());
            throw errors::MWSMessageException{err,global::get_line()};
        }

        for(size_t j = 0; j < alist.size(); ++j) {
            alist[j] = tools::check4placeholder(alist[j]);
        }

        std::vector<Variable> pargs;
        for(size_t j = 0; j < eve.params.size(); ++j) {
            try {
                if(!eve.params[j].matches(alist[j].to_variable())) {
                    std::string err_msg = "Invalid argument:\n\t- Expected: " + var_t2token(eve.params[j].type).content + "\n\t- But got: " + var_t2token(alist[j].to_variable().type).content;
                    throw errors::MWSMessageException(err_msg,global::get_line());
                }
                else {
                    ++global::in_argument_list;
                    pargs.push_back(alist[j].to_variable());
                    --global::in_argument_list;
                }
            }
            catch(errors::MWSMessageException& err) {
                throw err;
            }
            catch(...) {
                std::string err_msg = "Can't convert GeneralType " + general_t2token(alist[j].type).content + " to VariableType " + var_t2token(eve.params[j].type).content + " as function parameter for event: " + args[0].source.content;
                throw errors::MWSMessageException{err_msg,global::get_line()};
            }
        }

        for(auto i : eve.listeners) {
            i.run(pargs);
        }
        return general_null;
    }},
    {"listen",
        {
            car_Event,
            car_ArgumentList,
            car_Compound
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_NOT_BE_IN_STRUCT()
        if(!is_event(args[0].source.content)) {
            throw errors::MWSMessageException{"No such event to listen to: \"" + args[0].source.content + "\"",global::get_line()};
        }

        Event eve = global::events[args[0].source.content];
        if((eve.visibility != Event::Visibility::PUBLIC && eve.visibility != Event::Visibility::LISTEN_ONLY) && eve.from_file != global::include_path.top()) {
            throw errors::MWSMessageException{"Can't listen to inaccessable event: \"" + args[0].source.content + "\"",global::get_line()};
        }

        args[2].source.content.erase(args[2].source.content.begin());
        args[2].source.content.erase(args[2].source.content.begin() + args[2].source.content.size() - 1);
        std::string r = args[2].source.content;

        Function fun;
        fun.body = lex_text(r);
        fun.return_type = Variable::Type::VOID;
        fun.file = global::include_path.top();
        fun.scope_idx = get_new_scope();
        fun.params = tools::parse_function_params(args[1].source);
        scopes[fun.scope_idx].parent = current_scope()->index;

        global::events[args[0].source.content].listeners.push_back(fun);
        return general_null;
    }},
    
    {"struct",
        {
            car_Name,
            car_Compound,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_CAN_BE_IN_STRUCT()
        args[1].source.content.erase(args[1].source.content.begin());
        args[1].source.content.erase(args[1].source.content.begin()+args[1].source.content.size()-1);

        Object struc = construct_object(args[1]);
        if(!add_struct(args[0].source.content,struc)) {
            throw errors::MWSMessageException{"Can't redefine struct: " + args[0].source.content,global::get_line()};
        }
        return general_null;
    }},
    {"struct",
        {
            car_Name,
            car_ArgumentList,
            car_Compound,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_CAN_BE_IN_STRUCT()
        args[2].source.content.erase(args[2].source.content.begin());
        args[2].source.content.erase(args[2].source.content.begin()+args[2].source.content.size()-1);

        auto alist = tools::parse_argument_list(args[1]);
        std::vector<Object> structs;

        Object struc;

        for(auto i : alist) {
            if(!is_struct(i.to_string())) {
                throw errors::MWSMessageException{"Can't expand with unknown struct: " + i.to_string(),global::get_line()};
            }
            Object* st = get_struct(i.to_string());
            struc.members.insert(st->members.begin(),st->members.end());
            struc.methods.insert(st->methods.begin(),st->methods.end());
            struc.structs.insert(st->structs.begin(),st->structs.end());
        }

        Object cons = construct_object(args[2]);
        struc.members.insert(cons.members.begin(),cons.members.end());
        struc.methods.insert(cons.methods.begin(),cons.methods.end());
        struc.structs.insert(cons.structs.begin(),cons.structs.end());
        if(!add_struct(args[0].source.content,struc)) {
            throw errors::MWSMessageException{"Can't redefine struct: " + args[0].source.content,global::get_line()};
        }
        return general_null;
    }},
    {"gen_setget",
        {
            car_Name,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_BE_IN_STRUCT()
        Variable* var = get_variable(args[0].source.content);
        Function f = generate_get(args[0].source.content,*var);
        if(!add_function("get_" + args[0].source.content,f)) {
            throw errors::MWSMessageException{"Can't redefine function: " + args[0].source.content,global::get_line()};
        }
        f = generate_set(args[0].source.content,*var);
        if(!add_function("set_" + args[0].source.content,f)) {
            throw errors::MWSMessageException{"Can't redefine function: " + args[0].source.content,global::get_line()};
        }
        return general_null;
    }},
    {"gen_get",
        {
            car_Name,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_BE_IN_STRUCT()
        Variable* var = get_variable(args[0].source.content);
        Function f = generate_get(args[0].source.content,*var);
        if(!add_function("get_" + args[0].source.content,f)) {
            throw errors::MWSMessageException{"Can't redefine function: " + args[0].source.content,global::get_line()};
        }
        return general_null;
    }},
    {"gen_set",
        {
            car_Name,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_BE_IN_STRUCT()
        Variable* var = get_variable(args[0].source.content);
        Function f = generate_set(args[0].source.content,*var);
        if(!add_function("set_" + args[0].source.content,f)) {
            throw errors::MWSMessageException{"Can't redefine function: " + args[0].source.content,global::get_line()};
        }
        return general_null;
    }},
    {"on_death",
        {
            car_Function,
            car_ArgumentList,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        MWS_MUST_BE_IN_STRUCT()
        Object current = current_scope()->current_obj.top();
        argument_list alist = tools::parse_argument_list(args[1]);
        
        std::vector<Variable> vargs;
        for(auto i : alist) {
            vargs.push_back(tools::check4placeholder(i).to_variable()); //TODO: add catch for errors
        }

        Function* fptr = nullptr;
        for(auto& i : current_scope()->functions) {
            if(i.first == args[0].to_string()) {
                for(auto& j : i.second) { 
                    if(func_param_match(j,vargs)) {
                        fptr = &j;
                        goto OUT;
                    }
                }
            }
        }
OUT:

        if(fptr == nullptr && current_scope()->functions.count(args[0].to_string()) != 0) {
            std::string err_msg = "No overload of method " + args[0].to_string() + " matches agumentlist!\n- Got: [";
            for(auto i : vargs) {
                err_msg += var_t2token(i.type).content + ",";
            }
            err_msg.pop_back();
            throw errors::MWSMessageException{err_msg + "]",global::get_line()};
        }   

        if(current_scope()->functions.count(args[0].to_string()) == 0) {
            throw errors::MWSMessageException{"Only methods can be tagged as \"on_death\"!",global::get_line()};
        }
        current_scope()->current_obj.top().on_deconstruct.push_back(std::make_tuple(args[0].to_string(),vargs));
        return general_null;
    }},

};

std::vector<Command>* MeowScript::get_command_list() {
    return &commandlist;
}
