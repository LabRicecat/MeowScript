#ifndef MEOWSCRIPT_IG_LIST_HPP
#define MEOWSCRIPT_IG_LIST_HPP

#include "defs.hpp"
#include "reader.hpp"

#include <vector>
#include <unordered_map>

MEOWSCRIPT_HEADER_BEGIN

struct Variable;

struct List {
    std::vector<Variable> elements;

    static bool valid_list(Token context);

    std::string to_string() const;
};

List construct_list(Token context);

struct CommandArgReqirement;
struct GeneralTypeToken;

using MethodArgReqirement = CommandArgReqirement;
template<typename Self>
struct Method {
    std::string name;
    std::vector<MethodArgReqirement> args;

    Variable (*run)(std::vector<Variable> args, Self* self);
};

std::vector<Method<List>>* get_list_method_list();
Method<List>* get_list_method(std::string name);
bool is_list_method(std::string name);

std::vector<Method<Token>>* get_string_method_list();
Method<Token>* get_string_method(std::string name);
bool is_string_method(std::string name);

struct Dictionary;

std::vector<Method<Dictionary>>* get_dictionary_method_list();
Method<Dictionary>* get_dictionary_method(std::string name);
bool is_dictionary_method(std::string name);

MEOWSCRIPT_HEADER_END

#endif