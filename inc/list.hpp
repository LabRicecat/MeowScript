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
        bool tm = true;
        int in_br = false;
        bool in_q = false;
        bool bsh = false;
        for(auto i : context.content) {
            if(i == ',' && tm && in_br == 0 && !in_q) {
                return false;
            }
            else if(i == ',' && in_br == 0 && !in_q) {
                tm = true;
            }
            else if(i == '"' && in_br == 0 && !bsh) {
                in_q = !in_q;
                tm = false;
            }
            else if(is_open_brace(i) && !in_q) {
                ++in_br;
                tm = false;
            }
            else if(is_closing_brace(i) && !in_q) {
                --in_br;
                tm = false;
            }
            if(i == '\\') {
                if(bsh) {
                    tm = false;
                }
                bsh = true;
            }
            else {
                tm = false;
                bsh = false;
            }
        }
        if(in_q || in_br || tm) {
            return false;
        }
        return true;
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