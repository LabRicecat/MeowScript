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
            if(i == j) {
                return true;
            }
        }
    }
    return false;
}

bool MeowScript::CommandArgReqirement::matches(General_type type) {
    for(auto i : carry) {
        if(i == 0 || i == (static_cast<int>(type)+1)) {
            return true;
        }
    }
    return false;
}

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

static std::vector<Command> commandlist = {
    {"new",
        {
            car_Name,
            car_Number | car_String | car_List | car_PlaceHolderAble,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        new_variable(args[0].source.content,tools::check4placeholder(args[1]).to_variable());
        return general_null;
    }},
    {"set",
        {
            car_Name,
            car_Number | car_String | car_List | car_PlaceHolderAble,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        set_variable(args[0].source.content,tools::check4placeholder(args[1]).to_variable());
        return general_null;
    }},
    {"look",
        {
            car_Name
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        return GeneralTypeToken(*get_variable(args[0].source.content));
    }},
    {"return",
        {
            car_Any
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        auto ret = tools::check4placeholder(args[0]);
        ++global::runner_should_return;
        return ret;
    }},
    {"func",
        {
            car_Name,
            car_ArgumentList, 
            car_Operator, // ->
            car_Name, // ReturnValue
            car_Compound
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        Function fun;
        std::string ret_ty = args[3].to_string();
        args[4].source.content.erase(args[4].source.content.begin());
        args[4].source.content.erase(args[4].source.content.begin()+args[4].source.content.size()-1);
        fun.body = lex_text(args[4].source.content);
        
        args[1].source.content.erase(args[1].source.content.begin());
        args[1].source.content.erase(args[1].source.content.begin()+args[1].source.content.size()-1);

        if(args[2].to_string() != "->") {
            throw errors::MWSMessageException{"Expected \"->\" to declarate a return value, but got: " + args[2].to_string(),global::get_line()};
        }
        if(!is_valid_var_t(ret_ty) && ret_ty != "Any" && ret_ty != "Void") { // TODO: all classes from OOP later!
            throw errors::MWSMessageException{ret_ty + " is not a valid return value!",global::get_line()};
        }

        if(ret_ty == "Any") {
            fun.return_type = Variable::Type::ANY;
        }
        else if(ret_ty == "Void") {
            fun.return_type = Variable::Type::VOID;
        }
        else {
            fun.return_type = token2var_t(args[3].to_string());
        }

        auto lines = lex_text(args[1].source);
        std::vector<Token> line;
        for(auto i : lines) {
            for(auto j : i.source) {
                line.push_back(j);
            }
        }

        std::vector<Token> nline;
        for(size_t i = 0; i < line.size(); ++i) {
            nline.push_back("");
            for(size_t j = 0; j < line[i].content.size(); ++j) {
                if(line[i].content[j] == ',') {
                    nline.push_back(",");
                    nline.push_back("");
                }
                else {
                    nline.back().content += line[i].content[j];
                }
            }
        }

        std::vector<std::vector<Token>> arguments;
        if(nline.size() != 0)
            arguments.push_back({});
        for(size_t i = 0; i < nline.size(); ++i) {
            if(nline[i].content == ",") {
                if(arguments.back().empty()) {
                    throw errors::MWSMessageException{"Invalid argument format!",global::get_line()};
                }
                arguments.push_back({});
            }
            else {
                arguments.back().push_back(nline[i]);
            }
        }

        for(auto i : arguments) {
            for(size_t j = 0; j < i.size(); ++j) {
                if(i[j].content == "") {
                    i.erase(i.begin()+j);
                    --j;
                }
            }

            if(i.size() == 1) {
                if(!is_valid_name(i[0])) {
                    throw errors::MWSMessageException{"Enexpected token: " + i[0].content,global::get_line()};
                }
                fun.arg_names.push_back(i[0]);
                fun.args.push_back(Variable::Type::UNKNOWN);
            }
            else if(i.size() == 3) {
                if(!is_valid_name(i[0])) {
                    throw errors::MWSMessageException{"Enexpected token: " + i[0].content,global::get_line()};
                }
                if(i[1].content != "::") {
                    throw errors::MWSMessageException{"Invalid argument format!",global::get_line()};
                }
                if(!is_valid_var_t(i[2])) {
                    throw errors::MWSMessageException{i[2].content + " is not a known VariableType or class!",global::get_line()};
                }
                fun.arg_names.push_back(i[0]);
                fun.args.push_back(token2var_t(i[2]));
            }
            else {
                throw errors::MWSMessageException{"Invalid argument format!",global::get_line()};
            }
        }
        fun.file = global::include_path.top();
        if(!add_function(args[0].source.content,fun)) {
            throw errors::MWSMessageException{"Can't redefine function: " + args[0].source.content,global::get_line()};
        }
        return general_null;
    }},
    {"print",
        {
            car_Any
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        GeneralTypeToken gtt = tools::check4placeholder(args[0]);
        if(gtt.type == General_type::STRING) {
            std::cout << gtt.source.content << "\n";
        }
        else {
            std::cout << gtt.to_string() << "\n";
        }
        return general_null;
    }},
    {"if",
        {
            car_Expression,
            car_Compound,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        auto res = parse_expression(args[0].to_string());
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
        auto res = parse_expression(args[0].to_string());
        if(res.type != Variable::Type::Number) {
            throw errors::MWSMessageException{std::string("Expression \"" + args[0].to_string() + "\" did not return VariableType \"Number\" for if-statement!"),global::get_line()};
        }

        args[1].source.content.erase(args[1].source.content.begin());
        args[1].source.content.erase(args[1].source.content.begin()+args[1].source.content.size()-1);

        if(res.storage.number == 1 && !current_scope()->last_if_result) {
            auto ret = run_text(args[1].source.content,false,false);
            if(ret.type != General_type::VOID && ret.type != General_type::UNKNOWN) {
                ++global::runner_should_return;
                current_scope()->last_if_result = (res.storage.number == 1);
                return ret;
            }
        }
        else {
            current_scope()->last_if_result = true;
        }
        return general_null;
    }},
    {"else",
        {
            car_Compound,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
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
        if(args[0].type == General_type::MODULE && is_loaded_module(args[0].to_string()) && !get_module(args[0].to_string())->enabled) {
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
            car_Number | car_String | car_List | car_PlaceHolderAble,
            car_Compound
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
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
                auto ret = run_text(comp,false,false,-1,{{var_name,to_iterate.to_variable().storage.string.content[i]}},"",true);
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
        ++global::break_loop;
        return general_null;
    }},
    {"continue",
        {},
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        ++global::continue_loop;
        return general_null;
    }},
    {"typeof",
        {
            car_Any,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
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
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() > 1) {
            throw errors::MWSMessageException{"Too many/few arguments for command: input\n\t- Expected: <=1\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
        }
        if(alist.size() == 1) {
            auto output = tools::check4placeholder(alist[0]);
            if(output.type != General_type::STRING) {
                throw errors::MWSMessageException{"Wrong argument for argument!\n\t- Expected: String\n\t- But got: " + general_t2token(output.type).content,global::get_line()};
            }
            std::cout << output.source.content;
            std::cout.flush();
        }
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
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for cast: list()",global::get_line()};
        }
        alist[0] = tools::check4placeholder(alist[0]);
        switch(alist[0].type) {
            case General_type::STRING:
                {
                    List ret;
                    std::string iter = alist[0].to_variable().storage.string.content;
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

    {"import",
        {
            car_String,
        },
    [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
        std::filesystem::path pth = global::include_path.top().remove_filename().string() + args[0].source.content;
        std::filesystem::path pth2;
        
        if(!std::filesystem::exists(pth)) {
            pth2 = pth.string() + ".mws";
        }
        if(!std::filesystem::exists(pth2)) {
            throw errors::MWSMessageException{"Trying to import unknown file: \"" + pth.string() + "\"",global::get_line()};
        }

        return run_file(pth2,true,false,-1,{},pth2,false,true);
    }},
};

std::vector<Command>* MeowScript::get_command_list() {
    return &commandlist;
}