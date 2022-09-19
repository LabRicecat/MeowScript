#include "../inc/variables.hpp"
#include "../inc/list.hpp"
#include "../inc/commands.hpp"
#include "../inc/errors.hpp"
#include "../inc/global.hpp"
#include "../inc/functions.hpp"
#include "../inc/tools.hpp"
#include "../inc/modules.hpp"
#include "../inc/scopes.hpp"

#include <algorithm>

MEOWSCRIPT_SOURCE_FILE

Variable::Type MeowScript::general_t2var_t(General_type type) {
    switch(type) {
        case General_type::LIST:
            return Variable::Type::List;
        case General_type::NUMBER:
            return Variable::Type::Number;
        case General_type::STRING:
            return Variable::Type::String;
        case General_type::VOID:
            return Variable::Type::VOID;
        case General_type::DICTIONARY:
            return Variable::Type::Dictionary;
        default:
            std::string err = "Can't cast GeneralType " + general_t2token(type).content + " to VariableType!";
            throw errors::MWSMessageException(err,global::get_line());
    }
}

General_type MeowScript::var_t2general_t(Variable::Type type) {
    switch(type) {
        case Variable::Type::String:
            return General_type::STRING;
        case Variable::Type::List:
            return General_type::LIST;
        case Variable::Type::Number:
            return General_type::NUMBER;
        case Variable::Type::VOID:
            return General_type::VOID;
        default:
            return General_type::UNKNOWN;
    }
}

Variable MeowScript::make_variable(Token context, Variable::Type ty_) {
    Variable::Type type;
    if(ty_ != Variable::Type::UNKNOWN) {
        type = ty_;
    }
    else {
        type = general_t2var_t(get_type(context));
    }

    switch(type) {
        case Variable::Type::List:
            return Variable(construct_list(context));
        case Variable::Type::Number:
            if(is_capsuled_number(context)) {
                context.content.erase(context.content.begin());
                context.content.erase(context.content.begin()+context.content.size()-1);
            }
            return Variable(std::stod(tools::remove_uness_decs(context.content,false)));
        case Variable::Type::String:
            return Variable(context.content);
        case Variable::Type::Dictionary:
            {Variable ret; ret.type = Variable::Type::Dictionary; ret.storage.dict = dic_from_token(context); return ret;}
        case Variable::Type::VOID:
            {Variable ret; ret.type = type; return ret;}
    }
    return Variable();
}

Dictionary MeowScript::dic_from_token(Token context) {
    if(context.in_quotes || context.content == "") {
        return Dictionary{};
    }
    if(!brace_check(context,'{','}')) {
        return Dictionary{};
    }
    context.content.erase(context.content.begin());
    context.content.erase(context.content.end()-1);
    
    auto lines = lex_text(context.content);
    std::vector<Token> lexed;
    for(auto i : lines)
        for(auto j : i.source)
            lexed.push_back(j);
    
    if(lexed.size() == 0) {
        return Dictionary{};
    }

    GeneralTypeToken key;
    GeneralTypeToken value;
    bool got_equals = false;
    Dictionary ret;

    for(auto i : lexed) {
        if(!i.in_quotes && i.content == "=") {
            if(got_equals || key == general_null) {
                return Dictionary{};
            }
            got_equals = true;
        }
        else if(key == general_null) {
            key = GeneralTypeToken(i);
        }
        else if(got_equals && value == general_null) {
            value = GeneralTypeToken(i);
        }
        else if(!i.in_quotes && i.content == ",") {
            if(!got_equals || key == general_null || value == general_null) {
                return Dictionary{};
            }
            ret[key] = tools::check4placeholder(value);
            got_equals = false;
            key = general_null;
            value = general_null;
        }
        else {
            return Dictionary{};
        }
    }
    if(got_equals || key != general_null || value != general_null) {
        if(got_equals && key != general_null && value != general_null) {
            ret[key] = tools::check4placeholder(value);
        }
        else {
            return Dictionary{};
        }
    }
    return ret;
}

Token MeowScript::dic_to_token(Dictionary dic) {
    Token ret;
    ret.content = "{";
    for(auto i : dic.pairs()) {
        ret.content += i.first.to_string() + " = " + i.second.to_string() + ",";
    }
    ret.content.erase(ret.content.end()-1); // removes the last ","
    return ret.content + "}";
}

Token MeowScript::general_t2token(General_type type) {
    const std::string general_t_str[] = {
        "Number",
        "String",
        "List",
        "Function",
        "Command",
        "Compound",
        "Name",
        "Expression",
        "Argumentlist",
        "Operator",
        "Module",
        "Event",
        "Keyword",
        "Dictionary",
        "Unknown",
        "Void"
    };
    return general_t_str[static_cast<int>(type)];
}

Token MeowScript::var_t2token(Variable::Type type) {
    const std::string var_t_str[] = {
        "Number",
        "String",
        "List",
        "Dictionary",
        "UNKNOWN",
        "Any",
        "Void"
    };
    return var_t_str[static_cast<int>(type)];
}

Variable::Type MeowScript::token2var_t(Token token) {
    std::vector<std::string> var_t_str = {
        "Number",
        "String",
        "List",
        "Dictionary",
        "UNKNOWN",
        "Any",
        "Void"
    };
    for(size_t i = 0; i < var_t_str.size(); ++i) {
        if(var_t_str[i] == token.content) {
            return Variable::Type(i);
        }
    }
    return Variable::Type::UNKNOWN;
}

Variable MeowScript::GeneralTypeToken::to_variable() const {
    try {
        return make_variable(source,general_t2var_t(type));
    }
    catch(...) {
        std::string merr = "Can't cast GeneralType " + general_t2token(type).content + " to VariableType!";
        throw errors::MWSMessageException(merr,global::get_line());
    }
}

std::string MeowScript::GeneralTypeToken::to_string() const {
    switch(type) {
        case General_type::STRING:
        case General_type::LIST:
        case General_type::NUMBER:
        case General_type::DICTIONARY:
            return to_variable().to_string();
        case General_type::VOID:
            throw errors::MWSMessageException{"Can't cast GeneralType VOID to STRING",global::get_line()};
        default:
            return this->source.content;
    }
}

General_type MeowScript::get_type(Token context, CommandArgReqirement expected) {
    if(context.content == "" && !context.in_quotes) {
        return General_type::VOID;
    }
    if(all_numbers(context)) {
        return General_type::NUMBER;
    } 
    if(is_capsuled_number(context) && expected.matches(General_type::NUMBER)) {
        return General_type::NUMBER;
    }
    if(context.in_quotes || (context.content.front() == '"' && context.content.back() == '"')) {
        return General_type::STRING;
    }
    if(is_valid_argumentlist(context) && expected.matches(General_type::ARGUMENTLIST)) {
        return General_type::ARGUMENTLIST;
    }
    if(brace_check(context,'(',')') && expected.matches(General_type::EXPRESSION)) {
        return General_type::EXPRESSION;
    }
    if(is_dictionary(context) && expected.matches(General_type::DICTIONARY)) {
        return General_type::DICTIONARY;
    }
    if(brace_check(context,'{','}')) {
        return General_type::COMPOUND;
    }
    if(List::valid_list(context)) {
        return General_type::LIST;
    }
    if(is_command(context)) {
        return General_type::COMMAND;
    }
    if(is_function(context)) {
        return General_type::FUNCTION;
    }
    if(is_loaded_module(context.content)) {
        return General_type::MODULE;
    }
    if(is_event(context.content)) {
        return General_type::EVENT;
    }
    if(is_known_keyword(context)) {
        return General_type::KEYWORD;
    }
    if(is_valid_name(context)) {
        return General_type::NAME;
    }
    if(is_valid_operator_name(context)) {
        return General_type::OPERATOR;
    }
    return General_type::UNKNOWN;
}

std::string MeowScript::Variable::to_string() const {
    if(type == Variable::Type::Number) {
        return tools::remove_uness_decs(std::to_string(storage.number),false);
    }
    else if(type == Variable::Type::String) {
        return "\"" + storage.string.content + "\"";
    }
    else if(type == Variable::Type::List) {
        return storage.list.to_string();
    }
    else if(type == Variable::Type::Dictionary) {
        return dic_to_token(storage.dict);
    }
    else {
        return "";
    }
}

bool MeowScript::Variable::set(std::string str) {
    if((type != Variable::Type::String && fixed_type) || constant) {
        return false;
    }
    storage.string = str;
    type = Variable::Type::String;
    return true;
}
bool MeowScript::Variable::set(List list) {
    if((type != Variable::Type::List && fixed_type) || constant) {
        return false;
    }
    storage.list = list;
    type = Variable::Type::List;
    return true;
}
bool MeowScript::Variable::set(long double num) {
    if((type != Variable::Type::Number && fixed_type) || constant) {
        return false;
    }
    storage.number = num;
    type = Variable::Type::Number;
    return true;
}

std::string MeowScript::List::to_string() const {
    std::string ret = "[";
    for(size_t i = 0; i < elements.size(); ++i) {
        ret += (i == 0 ? "":",") + elements[i].to_string();
    }
    return ret + "]";
}

List MeowScript::construct_list(Token context) {
    List ret;
    if(context.content == "" || context.in_quotes || !brace_check(context,'[',']')) {
        return List();
    }
    context.content.erase(context.content.begin());
    context.content.erase(context.content.begin()+context.content.size()-1);
    if(context.content.size() == 0) {
        return List();
    }

    auto lx = lex_text(context.content);
    std::vector<Token> lexed;
    for(auto i : lx)
        for(auto j : i.source) 
            lexed.push_back(j);

    bool com = true;
    for(auto i : lexed) {
        if(i.content == "," && !i.in_quotes) {
            if(com) {
                throw errors::MWSMessageException{"Invalid list format!\n" + context.content,global::get_line()};
            }
            com = true;
        }
        else {
            if(!com) {
                throw errors::MWSMessageException{"Invalid list format!\n" + context.content,global::get_line()};
            }
            try {
                ret.elements.push_back(make_variable(i));
            }
            catch(std::exception& err) {
                //std::cout << "(" << err.what() << ")\n";
                throw errors::MWSMessageException{"Lists can only contain VariableType's!\nInvalid: " + i.content,global::get_line()};
            }
            com = false;
        }
    }
    if(com) {
        throw errors::MWSMessageException{"Invalid list format!\n" + context.content,global::get_line()};
    }

    return ret;
}

std::vector<Method<List>> list_method_list = {
    {"length",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        return self->elements.size();
    }},
    {"front",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        if(self->elements.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"at\" a on empty list!",global::get_line()};
        }
        return self->elements.front();
    }},
    {"back",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        if(self->elements.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"at\" a on empty list!",global::get_line()};
        }
        return self->elements.back();
    }},
    {"at",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        auto idx = tools::check4placeholder(alist[0]);
        if(idx.type != General_type::NUMBER) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\tBut got: " + general_t2token(idx.type).content,global::get_line()};
        }
        int index = idx.to_variable().storage.number;
        if(index < 0 || index >= self->elements.size()) {
            throw errors::MWSMessageException{"Index is not allowed to be less than 0 or bigger than the total size.",global::get_line()};
        }
        if(self->elements.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"at\" a on empty list!",global::get_line()};
        }
        return self->elements.at(index);
    }},
    {"insert",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 2) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 2\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        auto idx = tools::check4placeholder(alist[0]);
        if(idx.type != General_type::NUMBER) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\tBut got: " + general_t2token(idx.type).content,global::get_line()};
        }
        int index = idx.to_variable().storage.number;
        if(index < 0 || index >= self->elements.size()) {
            throw errors::MWSMessageException{"Index is not allowed to be less than 0 or bigger than the total size.",global::get_line()};
        }

        Variable value;
        try {
            value = tools::check4placeholder(args[1]).to_variable();
        }
        catch(...) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: [Number,String,List]\n\tBut got: " + general_t2token(args[1].type).content,global::get_line()};
        }
        
        if(self->elements.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"insert\" a on empty list!",global::get_line()};
        }
        self->elements.insert(self->elements.begin()+index,value);
        return *self;
    }},
    {"erase",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        auto idx = tools::check4placeholder(alist[0]);
        if(idx.type != General_type::NUMBER) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\tBut got: " + general_t2token(idx.type).content,global::get_line()};
        }
        int index = idx.to_variable().storage.number;
        if(index < 0 || index >= self->elements.size()) {
            throw errors::MWSMessageException{"Index is not allowed to be less than 0 or bigger than the total size.",global::get_line()};
        }
        if(self->elements.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"erase\" a on empty list!",global::get_line()};
        }
        self->elements.erase(self->elements.begin()+index);
        return *self;
    }},
    {"pop_front",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        if(self->elements.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"pop_front\" a on empty list!",global::get_line()};
        }
        self->elements.erase(self->elements.begin());
        return general_null;
    }},
    {"pop_back",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        if(self->elements.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"pop_back\" a on empty list!",global::get_line()};
        }
        self->elements.pop_back();
        return general_null;
    }},
    {"push_back",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        auto elem = tools::check4placeholder(alist[0]);
        if(!is_valid_var_t(general_t2token(elem.type))) {
            throw errors::MWSMessageException{"Invalid argument to push back!\n\t- Expected: [Number,String,List]\n\tBut got: " + general_t2token(elem.type).content,global::get_line()};
        }
        self->elements.push_back(elem.to_variable());
        return general_null;
    }},
    {"has",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        return get_list_method("count")->run(args,self).to_variable().storage.number != 0;
    }},
    {"count",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        auto elem = tools::check4placeholder(alist[0]);
        if(!is_valid_var_t(general_t2token(elem.type))) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: [Number,String,List]\n\tBut got: " + general_t2token(elem.type).content,global::get_line()};
        }
        auto f = elem.to_variable();
        size_t found = 0;
        for(auto i : self->elements) {
            if(i == f) {
                ++found;
            }
        }
        return found;
    }},
    {"empty",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        return self->elements.size() == 0;
    }},
    {"replace",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 2) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 2\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        auto idx = tools::check4placeholder(alist[0]);
        if(idx.type != General_type::NUMBER) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\tBut got: " + general_t2token(idx.type).content,global::get_line()};
        }
        int index = idx.to_variable().storage.number;
        if(index < 0 || index >= self->elements.size()) {
            throw errors::MWSMessageException{"Index is not allowed to be less than 0 or bigger than the total size.",global::get_line()};
        }

        Variable value;
        try {
            value = tools::check4placeholder(alist[1]).to_variable();
        }
        catch(...) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: [Number,String,List]\n\tBut got: " + general_t2token(args[1].type).content,global::get_line()};
        }
        
        if(self->elements.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"at\" a on empty list!",global::get_line()};
        }
        self->elements[index] = value;
        return general_null;
    }},

    {"sort",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        
        List nums;
        List strings;
        List lists;
        for(size_t i = 0; i < self->elements.size(); ++i) {
            if(self->elements[i].type == Variable::Type::Number) {
                if(nums.elements.empty()) {
                    nums.elements.push_back(self->elements[i]);
                }
                else {
                    bool f = false;
                    for(size_t j = 0; j < nums.elements.size(); ++j) {
                        if(nums.elements[j].storage.number <= self->elements[i].storage.number) {
                            nums.elements.insert(nums.elements.begin()+j,self->elements[i]);
                            f = true;
                            break;
                        }
                    }
                    if(!f) {
                        nums.elements.push_back(self->elements[i]);
                    }
                }
            }
            else if(self->elements[i].type == Variable::Type::String) {
                if(strings.elements.empty()) {
                    strings.elements.push_back(self->elements[i]);
                }
                else {
                    bool f = false;
                    for(size_t j = 0; j < strings.elements.size(); ++j) {
                        if(strings.elements[j].storage.string <= self->elements[i].storage.string) {
                            strings.elements.insert(strings.elements.begin()+j,self->elements[i]);
                            f = true;
                            break;
                        }
                    }
                    if(!f) {
                        strings.elements.push_back(self->elements[i]);
                    }
                }
            }
            else if(self->elements[i].type == Variable::Type::List) {
                if(lists.elements.empty()) {
                    lists.elements.push_back(self->elements[i]);
                }
                else {
                    bool f = false;
                    for(size_t j = 0; j < strings.elements.size(); ++j) {
                        if(lists.elements[j].storage.list.elements.size() <= self->elements[i].storage.list.elements.size()) {
                            lists.elements.insert(lists.elements.begin()+j,self->elements[i]);
                            f = true;
                            break;
                        }
                    }
                    if(!f) {
                        lists.elements.push_back(self->elements[i]);
                    }
                }
            }
        }

        self->elements = {};
        for(int i = nums.elements.size()-1; i != -1; --i) {
            self->elements.push_back(nums.elements[i]);
        }
        for(int i = strings.elements.size()-1; i != -1; --i) {
            self->elements.push_back(strings.elements[i]);
        }
        for(int i = lists.elements.size()-1; i != -1; --i) {
            self->elements.push_back(lists.elements[i]);
        }
        return general_null;
    }},
    {"sorted",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, List* self)->GeneralTypeToken {
        List n_list = *self;
        get_list_method("sort")->run(args,&n_list);
        return n_list;
    }},
};

std::vector<Method<List>>* MeowScript::get_list_method_list() {
    return &list_method_list;
}

Method<List>* MeowScript::get_list_method(std::string name) {
    for(auto& i : list_method_list) {
        if(i.name == name) {
            return &i;
        }
    }
    return nullptr;
}

bool MeowScript::is_list_method(std::string name) {
    return get_list_method(name) != nullptr;
}

std::vector<Method<Token>> string_method_list = {
    {"length",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for string method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        return self->content.size();
    }},
    {"front",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"at\" a on empty list!",global::get_line()};
        }
        return std::string(1,self->content.front());
    }},
    {"back",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for string method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"at\" a on empty string!",global::get_line()};
        }
        return std::string(1,self->content.back());
    }},
    {"at",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for string method!\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        auto idx = tools::check4placeholder(alist[0]);
        if(idx.type != General_type::NUMBER) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\tBut got: " + general_t2token(idx.type).content,global::get_line()};
        }
        int index = idx.to_variable().storage.number;
        if(index < 0 || index >= self->content.size()) {
            throw errors::MWSMessageException{"Index is not allowed to be less than 0 or bigger than the total size.",global::get_line()};
        }
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"at\" a on empty string!",global::get_line()};
        }
        return std::string(1,self->content.at(index));
    }},
    {"insert",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 2) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 2\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        auto idx = tools::check4placeholder(alist[0]);
        if(idx.type != General_type::NUMBER) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\tBut got: " + general_t2token(idx.type).content,global::get_line()};
        }
        int index = idx.to_variable().storage.number;
        if(index < 0 || index > self->content.size()) {
            throw errors::MWSMessageException{"Index is not allowed to be less than 0 or bigger than the total size.",global::get_line()};
        }

        std::string value;
        try {
            value = tools::check4placeholder(alist[1]).to_variable().storage.string;
        }
        catch(...) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: [Number,String,List]\n\tBut got: " + general_t2token(args[1].type).content,global::get_line()};
        }
        
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"insert\" a on empty list!",global::get_line()};
        }
        for(int i = value.size()-1; i != -1; --i) {
            self->content.insert(self->content.begin()+index,value[i]);
        }

        GeneralTypeToken ret;
        ret.source = *self;
        ret.type = General_type::STRING;
        return ret;
    }},
    {"erase",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for string method!\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        auto idx = tools::check4placeholder(alist[0]);
        if(idx.type != General_type::NUMBER) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\tBut got: " + general_t2token(idx.type).content,global::get_line()};
        }
        int index = idx.to_variable().storage.number;
        if(index < 0 || index >= self->content.size()) {
            throw errors::MWSMessageException{"Index is not allowed to be less than 0 or bigger than the total size.",global::get_line()};
        }
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"erase\" a on empty string!",global::get_line()};
        }
        self->content.erase(self->content.begin()+index);
        return *self;
    }},
    {"pop_front",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for string method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"pop_front\" a on empty string!",global::get_line()};
        }
        self->content.erase(self->content.begin());
        return general_null;
    }},
    {"pop_back",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for string method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"pop_back\" a on empty string!",global::get_line()};
        }
        self->content.pop_back();
        return general_null;
    }},
    {"push_back",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for string method!\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        auto elem = tools::check4placeholder(alist[0]);
        if(elem.type != General_type::STRING) {
            throw errors::MWSMessageException{"Invalid argument to push back!\n\t- Expected: String\n\tBut got: " + general_t2token(elem.type).content,global::get_line()};
        }
        self->content += elem.source.content;
        return general_null;
    }},
    {"has",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for string method!\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        auto elem = tools::check4placeholder(alist[0]);
        if(elem.type != General_type::STRING) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String\n\tBut got: " + general_t2token(elem.type).content,global::get_line()};
        }
        if(elem.source.content.size() != 1) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String (size:1)\n\tBut got: String (size:" + std::to_string(elem.source.content.size()) + ")",global::get_line()};
        }
        return get_string_method("count")->run(argument_list({elem}),self).to_variable().storage.number != 0;
    }},
    {"count",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for string method!\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        auto elem = tools::check4placeholder(alist[0]);
        if(elem.type != General_type::STRING) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String\n\tBut got: " + general_t2token(elem.type).content,global::get_line()};
        }
        if(elem.source.content.size() != 1) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String (size:1)\n\tBut got: String (size:" + std::to_string(elem.source.content.size()) + ")",global::get_line()};
        }
        auto f = elem.source.content[0];
        size_t found = 0;
        for(auto i : self->content) {
            if(i == f) {
                ++found;
            }
        }
        return found;
    }},
    {"empty",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for list method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        return self->content.size() == 0;
    }},
    {"replace",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 2) {
            throw errors::MWSMessageException{"Too many/few arguments for string method!\n\t- Expected: 2\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        auto idx = tools::check4placeholder(alist[0]);
        if(idx.type != General_type::NUMBER) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\tBut got: " + general_t2token(idx.type).content,global::get_line()};
        }
        int index = idx.to_variable().storage.number;
        if(index < 0 || index >= self->content.size()) {
            throw errors::MWSMessageException{"Index is not allowed to be less than 0 or bigger than the total size.",global::get_line()};
        }

        Variable value;
        try {
            value = tools::check4placeholder(alist[1]).to_variable();
            if(value.type != Variable::Type::String) {
                throw errors::MWSException{};
            }
        }
        catch(...) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String\n\tBut got: " + general_t2token(args[1].type).content,global::get_line()};
        }
        
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"replace\" a on empty list!",global::get_line()};
        }
        self->content = self->content.substr(0,index) + value.storage.string.content + self->content.substr(index+1,self->content.size()-1);
        return general_null;
    }},

    {"to_upper",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for string method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        for(auto& i : self->content) {
            i = toupper(i);
        }
        return general_null;
    }},
    {"to_lower",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Token* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for string method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        for(auto& i : self->content) {
            i = tolower(i);
        }
        return general_null;
    }},
};

std::vector<Method<Token>>* MeowScript::get_string_method_list() {
    return &string_method_list;
}

Method<Token>* MeowScript::get_string_method(std::string name) {
    for(auto& i : string_method_list) {
        if(i.name == name) {
            return &i;
        }
    }
    return nullptr;
}

bool MeowScript::is_string_method(std::string name) {
    return get_string_method(name) != nullptr;
}

std::vector<Method<Dictionary>> dictionary_method_list = {
    {"length",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Dictionary* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for dictionary method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        return self->keys().size();
    }},
    {"set",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Dictionary* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 2) {
            throw errors::MWSMessageException{"Too many/few arguments for dictionary method!\n\t- Expected: 2\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        self->operator[](tools::check4placeholder(alist[0])) = tools::check4placeholder(alist[1]);
        return general_null;
    }},
    {"get",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Dictionary* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 1) {
            throw errors::MWSMessageException{"Too many/few arguments for dictionary method!\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        return self->operator[](tools::check4placeholder(alist[0]));
    }},
    {"keys",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Dictionary* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for dictionary method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        List l;
        for(auto i : self->keys()) {
            try {
                l.elements.push_back(i.to_variable());
            }
            catch(...) {
                Variable v;
                v.type = Variable::Type::VOID;
                l.elements.push_back(v);
            }
        }

        return l;
    }},
    {"values",
    {
        car_ArgumentList,
    },
    [](std::vector<GeneralTypeToken> args, Dictionary* self)->GeneralTypeToken {
        auto alist = tools::parse_argument_list(args[0]);
        if(alist.size() != 0) {
            throw errors::MWSMessageException{"Too many/few arguments for dictionary method!\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()),global::get_line()};
        }
        List l;
        for(auto i : self->values()) {
            try {
                l.elements.push_back(i.to_variable());
            }
            catch(...) {
                Variable v;
                v.type = Variable::Type::VOID;
                l.elements.push_back(v);
            }
        }

        return l;
    }},
};

std::vector<Method<Dictionary>>* MeowScript::get_dictionary_method_list() {
    return &dictionary_method_list;
}

Method<Dictionary>* MeowScript::get_dictionary_method(std::string name) {
    for(auto& i : dictionary_method_list) {
        if(i.name == name) {
            return &i;
        }
    }
    return nullptr;
}

bool MeowScript::is_dictionary_method(std::string name) {
    return get_dictionary_method(name) != nullptr;
}