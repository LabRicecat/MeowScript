#ifndef MEOWSCRIPT_IG_LIST_HPP
#define MEOWSCRIPT_IG_LIST_HPP

#include "defs.hpp"
#include "reader.hpp"

#include <vector>

MEOWSCRIPT_HEADER_BEGIN

struct Variable;

struct List {
    std::vector<Variable> elements;

    static inline bool valid_list(Token context) {
        if(context.content == "" || context.in_quotes || !brace_check(context,'[',']')) {
            return false;
        }
        context.content.erase(context.content.begin());
        context.content.erase(context.content.begin()+context.content.size()-1);
        if(context.content.size() == 0) {
            return true;
        }
        
        context.content = "(" + context.content + ")";
        return is_valid_argumentlist(context.content);
    }

    std::string to_string();
};

List construct_list(Token context);

struct CommandArgReqirement;
struct GeneralTypeToken;

using MethodArgReqirement = CommandArgReqirement;
template<typename Self>
struct Method {
    std::string name;
    std::vector<MethodArgReqirement> args;

    GeneralTypeToken (*run)(std::vector<GeneralTypeToken> args, Self* self);
};

std::vector<Method<List>>* get_list_method_list();
Method<List>* get_list_method(std::string name);
bool is_list_method(std::string name);

std::vector<Method<Token>>* get_string_method_list();
Method<Token>* get_string_method(std::string name);
bool is_string_method(std::string name);


MEOWSCRIPT_HEADER_END

#endif