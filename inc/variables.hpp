#ifndef MEOWSCRIPT_IG_VARIABLES_HPP
#define MEOWSCRIPT_IG_VARIABLES_HPP

#include "defs.hpp"
#include "reader.hpp"
#include "list.hpp"
#include "commands.hpp"
#include "tools.hpp"

#include <unordered_map>

MEOWSCRIPT_HEADER_BEGIN

General_type get_type(Token context, CommandArgReqirement expected = car_Any);

struct GeneralTypeToken;

struct Dictionary {
private:
    std::vector<GeneralTypeToken> i_keys;
    std::vector<GeneralTypeToken> i_values;
public:
    const std::vector<std::pair<GeneralTypeToken,GeneralTypeToken>> pairs() const;
    std::vector<std::pair<GeneralTypeToken,GeneralTypeToken>> pairs();

    const std::vector<GeneralTypeToken>& keys() const;
    const std::vector<GeneralTypeToken>& values() const;
    std::vector<GeneralTypeToken>& keys();
    std::vector<GeneralTypeToken>& values();

    bool has(const GeneralTypeToken gtt) const;

    GeneralTypeToken& operator[](GeneralTypeToken gtt);
};

Dictionary dic_from_token(Token tk);
Token dic_to_token(Dictionary dic);

struct Variable {
private:
    struct stor_ {
        long double number = 0.0;
        Token string;
        List list;
        Dictionary dict;
    };
public:
    enum class Type {
        Number,
        String,
        List,
        Dictionary,
        UNKNOWN, // Dont use
        ANY, // Also don't use
        VOID, // Guess what
    }type = Type::UNKNOWN;

    stor_ storage;

    Variable(List list) {type = Type::List; storage.list = list;}
    Variable(Token string) {type = Type::String; storage.string = string; storage.string.in_quotes = true;}
    Variable(long double number) {type = Type::Number; storage.number = number;}
    Variable(Dictionary dic) {type = Variable::Type::Dictionary, storage.dict = dic;}
    Variable() {}

    std::string to_string() const;

    bool fixed_type = false;
    bool constant = false;

    // all `set` return false if they fail because of `fixed_type` or `constant`
    bool set(std::string str);
    bool set(List list);
    bool set(long double num);

    bool operator==(Variable v) {
        return this->type == v.type && this->to_string() == v.to_string();
    }
};

General_type var_t2general_t(Variable::Type type);

struct GeneralTypeToken {
    Token source;
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
            default:
                type = General_type::VOID;
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
};

namespace tools {
    std::tuple<std::vector<std::string>,std::vector<Variable::Type>> parse_function_params(Token context);

    inline std::map<std::string,GeneralTypeToken> replaces = {
        {"pi",GeneralTypeToken(3.14159)},
        {"phi",GeneralTypeToken(1.61803)},
        {"e",GeneralTypeToken(2.71828)}
    };

    GeneralTypeToken check4replace(GeneralTypeToken token);
    Token check4replace(Token token);
}

inline const GeneralTypeToken general_null = GeneralTypeToken{};

// Conversion functions
Variable::Type general_t2var_t(General_type type);
Token general_t2token(General_type type);
Token var_t2token(Variable::Type type);
Variable::Type token2var_t(Token token);

// constructs a variable based of the given context
Variable make_variable(Token context, Variable::Type ty_ = Variable::Type::UNKNOWN);

MEOWSCRIPT_HEADER_END

#endif