#include "../inc/variables.hpp"
#include "../inc/list.hpp"
#include "../inc/commands.hpp"
#include "../inc/errors.hpp"
#include "../inc/global.hpp"
#include "../inc/functions.hpp"
#include "../inc/tools.hpp"
#include "../inc/modules.hpp"
#include "../inc/scopes.hpp"
#include "../inc/expressions.hpp"
#include "../inc/reader.hpp"

#include <algorithm>

MEOWSCRIPT_SOURCE_FILE

Variable::Variable(Function func) {
    type = Variable::Type::Function;
    storage.func.push_back(func);
}

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
        case General_type::OBJECT:
            return Variable::Type::Object;
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
        case Variable::Type::Object:
            return General_type::OBJECT;
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
        //case Variable::Type::Object: 
        //    return *get_object(context.content);
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
            ret[tools::check4placeholder(key).to_variable()] = tools::check4placeholder(value).to_variable();
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
            ret[tools::check4placeholder(key).to_variable()] = tools::check4placeholder(value).to_variable();
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
        "Parameterlist",
        "Operator",
        "Module",
        "Event",
        "Keyword",
        "Dictionary",
        "Struct",
        "Object",
        "Typename",
        "Unknown",
        "Void",
        "FUNCCALL",
    };
    return general_t_str[static_cast<int>(type)];
}

Token MeowScript::var_t2token(Variable::Type type) {
    const std::string var_t_str[] = {
        "Number",
        "String",
        "List",
        "Dictionary",
        "Object",
        "Function",
        "UNKNOWN",
        "Any",
        "Void",
        "FUNCCALL",
    };
    return var_t_str[static_cast<int>(type)];
}

Variable::Type MeowScript::token2var_t(Token token) {
    std::vector<std::string> var_t_str = {
        "Number",
        "String",
        "List",
        "Dictionary",
        "Object",
        "Function",
        "UNKNOWN",
        "Any",
        "Void",
        "FUNCCALL",
    };
    for(size_t i = 0; i < var_t_str.size(); ++i) {
        if(var_t_str[i] == token.content) {
            return Variable::Type(i);
        }
    }
    return Variable::Type::UNKNOWN;
}

Variable MeowScript::GeneralTypeToken::to_variable() const {
    if(type == General_type::FUNCCALL) {
        Variable ret;
        ret.type = Variable::Type::FUNCCALL;
        ret.storage.function_call = funccall;
        return ret;
    }
    else if(type == General_type::OBJECT) {
        Variable ret;
        if(use_save_obj) {
            ret.storage.obj = saveobj;
            ret.type = Variable::Type::Object;
            return ret;
        }
        Object* obj = get_object(source.content);
        ret.type = Variable::Type::Object;
        ret.storage.obj = *obj;
        return ret;
    }
    else if(type == General_type::FUNCTION) {
        Variable ret;
        ret.type = Variable::Type::Function;
        if(is_function_literal(source)) {
            ret.storage.func = functions_from_string(source.content);
        }
        else {
            ret.storage.func = get_function_overloads(source.content);
        }
        return ret;
    }
    try {
        return make_variable(source,general_t2var_t(type));
    }
    catch(errors::MWSMessageException& err) {
        std::string merr = "Can't cast GeneralType " + general_t2token(type).content + " (\"" + this->source.content + "\") to VariableType!";
        throw errors::MWSMessageException(merr,global::get_line());
    }
}

std::string MeowScript::GeneralTypeToken::to_string() const {
    switch(type) {
        case General_type::STRING:
        case General_type::LIST:
        case General_type::NUMBER:
        case General_type::DICTIONARY:
        case General_type::FUNCCALL:
            return to_variable().to_string();
        case General_type::VOID:
            throw errors::MWSMessageException{"Can't cast GeneralType VOID to STRING",global::get_line()};
        case General_type::FUNCTION:
            return source.content;
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
    if(is_expression(context.content) && expected.matches(General_type::EXPRESSION)) {
        return General_type::EXPRESSION;
    }
    if(is_valid_parameterlist(context) && expected.matches(General_type::PARAMETERLIST)) {
        return General_type::PARAMETERLIST;
    }
    if(is_valid_argumentlist(context) && expected.matches(General_type::ARGUMENTLIST)) {
        return General_type::ARGUMENTLIST;
    }
    if(is_dictionary(context) && expected.matches(General_type::DICTIONARY)) {
        return General_type::DICTIONARY;
    }
    if(is_function_call(context) && expected.matches(General_type::FUNCCALL)) {
        return General_type::FUNCCALL;
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
    if(is_function_literal(context) && expected.matches(General_type::FUNCTION)) {
        return General_type::FUNCTION;
    }
    if(is_valid_var_t(context) || is_funcparam_literal(context)) {
        return General_type::TYPENAME;
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
    if(is_struct(context.content)) {
        return General_type::STRUCT;
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
    else if(type == Variable::Type::Function) {
        return funcoverloads_to_string(storage.func);
    }
    else if(type == Variable::Type::FUNCCALL) {
        std::string r;
        r += storage.function_call.func;
        r += "(";
        for(auto i : storage.function_call.arglist) {
            r += i.to_string() + ",";
        }
        if(r.back() != '(') r.pop_back();
        r += ")";
        if(storage.function_call.shadow_return) r += "!";
        return r;
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
    if((type != Variable::Type::ANY && type != Variable::Type::Number && fixed_type) || constant) {
        return false;
    }
    storage.number = num;
    type = Variable::Type::Number;
    return true;
}

bool MeowScript::Variable::set(Variable var) {
    if((type != Variable::Type::ANY && type != var.type && fixed_type) || constant) {
        return false;
    }
    storage = var.storage;
    type = var.type;
    return true;
}

bool MeowScript::matches(Variable::Type type1, General_type type2) {
    /*if(type2 == General_type::STRUCT || type1 > Variable::Type::OUT_OF_RANGE) {
        return ( 
            (type2 == General_type::OBJECT && type1.struc_name == type2.struc_name) 
                || 
            (type1 == Variable::Type::Object && type1.struc_name == type2.struc_name)
            ); 
    }*/
    return var_t2general_t(type1) == type2;
}

bool matches(std::vector<Variable::Type> types, General_type gtype) {
    for(auto i : types) {
        if(matches(i,gtype)) {
            return true;
        }
    }
    return false;
}

bool MeowScript::List::valid_list(Token context) {
    if(context.content == "" || context.in_quotes || !brace_check(context,'[',']')) {
        return false;
    }
    context.content.erase(context.content.begin());
    context.content.erase(context.content.begin()+context.content.size()-1);
    if(context.content.size() == 0) {
        return true;
    }
        
    auto l = lex_text(context.content);
    std::vector<Token> tokens;
    for(auto i : l)
        for(auto j : i.source)
            tokens.push_back(j);

    std::string last;
    bool found_con = false;

    for(auto i : tokens) {
        if(i.content == "," && !i.in_quotes) {
            if(!found_con) return false;
            found_con = false;
        }
        else {
            if(found_con) return false;
            found_con = true;
        }
    }
    
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
                ret.elements.push_back(tools::check4placeholder(i).to_variable());
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
        
    },
    [](std::vector<Variable> args, List* self)->Variable {
        return self->elements.size();
    }},
    {"front",
    {
    },
    [](std::vector<Variable> args, List* self)->Variable {
        if(self->elements.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"at\" a on empty list!",global::get_line()};
        }
        return self->elements.front();
    }},
    {"back",
    {
    },
    [](std::vector<Variable> args, List* self)->Variable {
        if(self->elements.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"at\" a on empty list!",global::get_line()};
        }
        return self->elements.back();
    }},
    {"at",
    {
        car_Number,
    },
    [](std::vector<Variable> args, List* self)->Variable {
        int index = args[0].storage.number;
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
        car_Number,
        car_Number | car_String | car_List | car_Object | car_Dictionary,
    },
    [](std::vector<Variable> args, List* self)->Variable {
        auto idx = args[0];
        int index = idx.storage.number;
        if(index < 0 || index >= self->elements.size()) {
            throw errors::MWSMessageException{"Index is not allowed to be less than 0 or bigger than the total size.",global::get_line()};
        }

        Variable value;
        value = args[1];
        
        if(self->elements.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"insert\" a on empty list!",global::get_line()};
        }
        self->elements.insert(self->elements.begin()+index,value);
        return *self;
    }},
    {"erase",
    {
        car_Number,
    },
    [](std::vector<Variable> args, List* self)->Variable {
        int index = args[0].storage.number;
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

    },
    [](std::vector<Variable> args, List* self)->Variable {
        if(self->elements.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"pop_front\" a on empty list!",global::get_line()};
        }
        self->elements.erase(self->elements.begin());
        return general_null;
    }},
    {"pop_back",
    {

    },
    [](std::vector<Variable> args, List* self)->Variable {
        if(self->elements.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"pop_back\" a on empty list!",global::get_line()};
        }
        self->elements.pop_back();
        return general_null;
    }},
    {"push_back",
    {
        car_Number | car_String | car_List | car_Object | car_Dictionary,
    },
    [](std::vector<Variable> args, List* self)->Variable {
        auto elem = args[0];
        self->elements.push_back(elem);
        return general_null;
    }},
    {"has",
    {

    },
    [](std::vector<Variable> args, List* self)->Variable {
        return get_list_method("count")->run(args,self).storage.number != 0;
    }},
    {"count",
    {
        car_Number | car_String | car_List | car_Object | car_Dictionary
    },
    [](std::vector<Variable> args, List* self)->Variable {
        auto elem = args[0];
        size_t found = 0;
        for(auto i : self->elements) {
            if(i == elem) {
                ++found;
            }
        }
        return found;
    }},
    {"empty",
    {

    },
    [](std::vector<Variable> args, List* self)->Variable {
        return self->elements.size() == 0;
    }},
    {"replace",
    {
        car_Number,
        car_Number | car_String | car_List | car_Object | car_Dictionary
    },
    [](std::vector<Variable> args, List* self)->Variable {
        int index = args[0].storage.number;

        Variable value;
        value = args[1];
        
        if(self->elements.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"at\" a on empty list!",global::get_line()};
        }
        self->elements[index] = value;
        return general_null;
    }},

    {"sort",
    {

    },
    [](std::vector<Variable> args, List* self)->Variable {
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

    },
    [](std::vector<Variable> args, List* self)->Variable {
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

    },
    [](std::vector<Variable> args, Token* self)->Variable {
        return self->content.size();
    }},
    {"front",
    {

    },
    [](std::vector<Variable> args, Token* self)->Variable {
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"at\" a on empty list!",global::get_line()};
        }
        Token tk;
        tk.in_quotes = true;
        tk.content = std::string(1,self->content.front());
        return tk;
    }},
    {"back",
    {
    },
    [](std::vector<Variable> args, Token* self)->Variable {
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"at\" a on empty string!",global::get_line()};
        }
        Token tk;
        tk.in_quotes = true;
        tk.content = std::string(1,self->content.back());
        return tk;
    }},
    {"at",
    {
        car_Number,
    },
    [](std::vector<Variable> args, Token* self)->Variable {
        int index = args[0].storage.number;
        if(index < 0 || index >= self->content.size()) {
            throw errors::MWSMessageException{"Index is not allowed to be less than 0 or bigger than the total size.",global::get_line()};
        }
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"at\" a on empty string!",global::get_line()};
        }
        Token tk;
        tk.in_quotes = true;
        tk.content = std::string(1,self->content.at(index));
        return tk;
    }},
    {"insert",
    {
        car_Number,
        car_String,
    },
    [](std::vector<Variable> args, Token* self)->Variable {
        int index = args[0].storage.number;
        if(index < 0 || index > self->content.size()) {
            throw errors::MWSMessageException{"Index is not allowed to be less than 0 or bigger than the total size.",global::get_line()};
        }

        std::string value;
        value = args[1].storage.string;
        
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"insert\" a on empty string!",global::get_line()};
        }
        for(int i = value.size()-1; i != -1; --i) {
            self->content.insert(self->content.begin()+index,value[i]);
        }

        Variable ret;
        ret.storage.string = *self;
        ret.type = Variable::Type::String;
        return ret;
    }},
    {"erase",
    {
        car_Number,
    },
    [](std::vector<Variable> args, Token* self)->Variable {
        int index = args[0].storage.number;
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
    },
    [](std::vector<Variable> args, Token* self)->Variable {
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"pop_front\" a on empty string!",global::get_line()};
        }
        self->content.erase(self->content.begin());
        return general_null;
    }},
    {"pop_back",
    {

    },
    [](std::vector<Variable> args, Token* self)->Variable {
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"pop_back\" a on empty string!",global::get_line()};
        }
        self->content.pop_back();
        return general_null;
    }},
    {"push_back",
    {
        car_String,
    },
    [](std::vector<Variable> args, Token* self)->Variable {
        auto elem = args[0].storage.string;
        self->content += elem;
        return general_null;
    }},
    {"has",
    {
        car_String,
    },
    [](std::vector<Variable> args, Token* self)->Variable {
        // TODO: improve
        return get_string_method("count")->run(args,self).storage.number != 0;
    }},
    {"count",
    {
        car_String,
    },
    [](std::vector<Variable> args, Token* self)->Variable {
        auto elem = args[0].storage.string;
        if(elem.content.size() != 1) {
            throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String (size:1)\n\tBut got: String (size:" + std::to_string(elem.content.size()) + ")",global::get_line()};
        }
        auto f = elem.content[0];
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
    },
    [](std::vector<Variable> args, Token* self)->Variable {
        return self->content.size() == 0;
    }},
    {"replace",
    {
        car_Number,
        car_String,
    },
    [](std::vector<Variable> args, Token* self)->Variable {
        int index = args[0].storage.number;
        if(index < 0 || index >= self->content.size()) {
            throw errors::MWSMessageException{"Index is not allowed to be less than 0 or bigger than the total size.",global::get_line()};
        }

        Variable value;
        value = args[1];
        
        if(self->content.size() == 0) {
            throw errors::MWSMessageException{"Can't call method \"replace\" a on empty list!",global::get_line()};
        }
        self->content = self->content.substr(0,index) + value.storage.string.content + self->content.substr(index+1,self->content.size()-1);
        return general_null;
    }},

    {"to_upper",
    {
    },
    [](std::vector<Variable> args, Token* self)->Variable {
        for(auto& i : self->content) {
            i = toupper(i);
        }
        return general_null;
    }},
    {"to_lower",
    {

    },
    [](std::vector<Variable> args, Token* self)->Variable {
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
    [](std::vector<Variable> args, Dictionary* self)->Variable {
        return self->keys().size();
    }},
    {"set",
    {
        car_String | car_Number | car_List | car_Dictionary,
        car_String | car_Number | car_List | car_Dictionary,
    },
    [](std::vector<Variable> args, Dictionary* self)->Variable {
        self->operator[](args[0]) = args[1];
        return general_null;
    }},
    {"get",
    {
        car_String | car_Number | car_List | car_Dictionary,
    },
    [](std::vector<Variable> args, Dictionary* self)->Variable {
        return self->operator[](args[0]);
    }},
    {"keys",
    {

    },
    [](std::vector<Variable> args, Dictionary* self)->Variable {
        List l;
        for(auto i : self->keys()) {
            try {
                l.elements.push_back(i);
            }
            catch(...) {
                Variable v;
                v.type = Variable::Type::String;
                v.storage.string.content = i.to_string();
                v.storage.string.in_quotes = true;
                l.elements.push_back(v);
            }
        }

        return l;
    }},
    {"values",
    {
        
    },
    [](std::vector<Variable> args, Dictionary* self)->Variable {
        List l;
        for(auto i : self->values()) 
            l.elements.push_back(i);

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


Variable::FunctionCall MeowScript::parse_function_call(Token context) {
    if(context.in_quotes) return Variable::FunctionCall{};
    KittenLexer lexer = KittenLexer()
        .add_capsule('(',')')
        .add_capsule('{','}')
        .add_capsule('[',']')
        .add_con_extract(is_valid_operator_char)
        .add_ignore(' ')
        .add_ignore('\n')
        .erase_empty()
        ;
    auto lexed = lexer.lex(context.content);
    if(lexed.size() != 3 && lexed.size() != 2) return Variable::FunctionCall{};
    if(!is_valid_name(lexed[0].src) && !is_function_literal(lexed[0].src)) return Variable::FunctionCall{};
    if(!is_valid_argumentlist(lexed[1].src)) return Variable::FunctionCall{};
    if(!((lexed.size() == 3 && lexed[2].src == "!" && !lexed[2].str) || lexed.size() == 2))) return Variable::FunctionCall{};

    Variable::FunctionCall ret;
    ret.func = lexed[0].src;
    if(is_function_literal(lexed[0].src)) {
        ret.state = 1;
    }
    else if(is_variable(lexed[0].src)) {
        ret.state = 2; 
    }
    else {
        ret.state = 0;
    }

    ret.arglist = tools::parse_argument_list((Token)lexed[1].src);
    if(lexed.size() == 3) {
        ret.shadow_return = true;
    }
    return ret;
}