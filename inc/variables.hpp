#ifndef MEOWSCRIPT_IG_VARIABLES_HPP
#define MEOWSCRIPT_IG_VARIABLES_HPP

#include "defs.hpp"
#include "reader.hpp"
#include "list.hpp"
#include "commands.hpp"
#include "tools.hpp"
#include "objects.hpp"
#include <unordered_map>

MEOWSCRIPT_HEADER_BEGIN

General_type get_type(Token context, CommandArgReqirement expected = car_Any);

struct GeneralTypeToken;

struct Dictionary {
private:
    std::vector<Variable> i_keys;
    std::vector<Variable> i_values;
public:
    const std::vector<std::pair<Variable,Variable>> pairs() const;
    std::vector<std::pair<Variable,Variable>> pairs();

    const std::vector<Variable>& keys() const;
    const std::vector<Variable>& values() const;
    std::vector<Variable>& keys();
    std::vector<Variable>& values();

    bool has(const Variable key) const;

    Variable& operator[](Variable key);
};

Dictionary dic_from_token(Token tk);
Token dic_to_token(Dictionary dic);

struct Function;
struct Variable {
    struct FunctionCall { 
        std::string func; 
        argument_list arglist; 
        bool shadow_return = false; 

        // 0 => function name
        // 1 => lambda
        // 2 => var
        int state = 0;
    };
private:
    struct stor_ {
        long double number = 0.0;
        Token string;
        List list;
        Dictionary dict;
        Object obj;
        std::vector<Function> func;
        FunctionCall function_call;
    };
public:
    enum class Type {
        Number,
        String,
        List,
        Dictionary,
        Object,
        Function,
        UNKNOWN, // Dont use
        ANY, // Also don't use
        VOID, // Guess what
        FUNCCALL, // same here
        OUT_OF_RANGE,
    }type = Type::UNKNOWN;
    std::string struct_name;

    stor_ storage;

    Variable(List list) {type = Type::List; storage.list = list;}
    Variable(Token string) {type = Type::String; storage.string = string; storage.string.in_quotes = true;}
    Variable(long double number) {type = Type::Number; storage.number = number;}
    Variable(Dictionary dic) {type = Variable::Type::Dictionary; storage.dict = dic;}
    Variable(Object obj) {type = Variable::Type::Object; storage.obj = obj;}
    Variable(FunctionCall funcc) {type = Variable::Type::FUNCCALL; storage.function_call = funcc; }
    Variable(Type ty) : type(ty) {}
    Variable(Function func);
    explicit Variable(std::string str) {type = Variable::Type::String; storage.string = str; storage.string.in_quotes = true;}
    explicit Variable(const char* str) {type = Variable::Type::String; storage.string = std::string(str); storage.string.in_quotes = true;}
    Variable() {}

    std::string to_string() const;

    bool fixed_type = false;
    bool constant = false;

    // all `set` return false if they fail because of `fixed_type` or `constant`
    bool set(std::string str);
    bool set(List list);
    bool set(long double num);
    bool set(Variable var);

    bool operator==(Variable v) const {
        if(this->type == Variable::Type::Object && v.type == Variable::Type::Object) {
            Object a = storage.obj;
            return struct_matches(&a,&v.storage.obj);
        }
        return this->type == v.type && this->to_string() == v.to_string();
    }

    bool operator==(Variable v) {
        if(this->type == Variable::Type::Object && v.type == Variable::Type::Object) {
            return struct_matches(&storage.obj,&v.storage.obj);
        }
        return this->type == v.type && this->to_string() == v.to_string();
    }
    bool operator!=(Variable v) {
        return !operator==(v);
    }
};

bool matches(Variable::Type type1, General_type type2);
bool matches(std::vector<Variable::Type> types, General_type gtype);

General_type var_t2general_t(Variable::Type type);
Variable::FunctionCall parse_function_call(Token context);

struct GeneralTypeToken {
    Token source;
    Variable::FunctionCall funccall;
    General_type type = General_type::VOID;

    GeneralTypeToken(Token context) {
        source = context;
        type = get_type(context);
        if(!source.in_quotes && type == General_type::STRING) {
            source.content.erase(source.content.begin());
            source.content.erase(source.content.begin()+source.content.size()-1);
            source = tools::remove_unneeded_chars(source);
            source.in_quotes = true;
        }
        else if(type == General_type::FUNCCALL) {
            funccall = parse_function_call(context);
        }
    }
    GeneralTypeToken(Variable var) {
        switch(var.type) {
            case Variable::Type::Number:
                source = std::to_string(var.storage.number);
                type = General_type::NUMBER;
                break;
            case Variable::Type::String:
                source = var.storage.string;
                type = General_type::STRING;
                break;
            case Variable::Type::List:
                source = var.storage.list.to_string();
                type = General_type::LIST;
                break;
            case Variable::Type::Dictionary:
                source = dic_to_token(var.storage.dict);
                type = General_type::DICTIONARY;
                break;
            case Variable::Type::Object:
                source = (std::string)"#OBJECT#";
                type = General_type::OBJECT;
                use_save_obj = true;
                saveobj = var.storage.obj;
                break;
            case Variable::Type::Function:
                source = var.to_string();
                type = General_type::FUNCTION;
                break;
            case Variable::Type::FUNCCALL:
                funccall = var.storage.function_call;
                type = General_type::FUNCCALL;
                break;
            default:
                type = General_type::VOID;
        }
    }
    GeneralTypeToken(Token token, CommandArgReqirement cars) {
        type = get_type(token,cars);
        source = token;
        if(!source.in_quotes && type == General_type::STRING) {
            source.content.erase(source.content.begin());
            source.content.erase(source.content.begin()+source.content.size()-1);
            source = tools::remove_unneeded_chars(source);
            source.in_quotes = true;
        }
        else if(type == General_type::FUNCCALL) {
            funccall = parse_function_call(context);
        }
    }
    GeneralTypeToken() {}

    Variable to_variable() const;
    std::string to_string() const;

    // casts
    GeneralTypeToken(long double i) {
        source = std::to_string(i);
        source.in_quotes = false;
        type = General_type::NUMBER;
    }
    GeneralTypeToken(List list) {
        source = list.to_string();
        source.in_quotes = false;
        type = General_type::LIST;
    }
    GeneralTypeToken(const char* str) {
        source = std::string(str);
        source.in_quotes = true;
        type = General_type::STRING;
    }
    GeneralTypeToken(std::string str) {
        source = str;
        source.in_quotes = true;
        type = General_type::STRING;
    }
    GeneralTypeToken(Dictionary dict) {
        source = dic_to_token(dict);
        source.in_quotes = false;
        type = General_type::DICTIONARY;
    }

    bool operator==(GeneralTypeToken gtt) {
        /*if(gtt.type != General_type::OBJECT) {
            return gtt.type == type && gtt.source == source;
        }*/
        return gtt.type == type && gtt.source == source;
    }
    bool operator==(const GeneralTypeToken gtt) const {
        return gtt.type == type && gtt.source == source;
    }
    bool operator!=(GeneralTypeToken gtt) {
        return !operator==(gtt);
    }
    bool operator!=(const GeneralTypeToken gtt) const {
        return !operator==(gtt);
    }

    bool use_save_obj = false;
    bool nameless_function = false;
    Object saveobj;
};

struct Parameter;
namespace tools {
    std::vector<Parameter> parse_function_params(Token context);

    inline std::map<std::string,GeneralTypeToken> replaces = {
        {"pi",GeneralTypeToken(3.14159)},
        {"phi",GeneralTypeToken(1.61803)},
        {"e",GeneralTypeToken(2.71828)},
        {"true",GeneralTypeToken(1)},
        {"false",GeneralTypeToken("0",car_Number)}
    };

    GeneralTypeToken check4replace(GeneralTypeToken token);
    Token check4replace(Token token);
}

inline const Variable general_null = Variable{};

// Conversion functions
Variable::Type general_t2var_t(General_type type);
Token general_t2token(General_type type);
Token var_t2token(Variable::Type type);
Variable::Type token2var_t(Token token);

// constructs a variable based of the given context
Variable make_variable(Token context, Variable::Type ty_ = Variable::Type::UNKNOWN);

MEOWSCRIPT_HEADER_END

#endif