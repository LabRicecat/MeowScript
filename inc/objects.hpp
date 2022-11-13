#ifndef MEOWSCRIPT_IG_OBJECTS_HPP
#define MEOWSCRIPT_IG_OBJECTS_HPP

#include "defs.hpp"
#include "reader.hpp"

#include <map>
#include <vector>
#include <string>

MEOWSCRIPT_HEADER_BEGIN

struct Variable;
struct Function;
struct GeneralTypeToken;

struct Object {
    int parent_scope = 0;

    std::map<std::string,Variable> members;
    std::map<std::string,Function> methods;
    std::map<std::string,Object> structs;
};

Function generate_get(Token name, Variable member);
Function generate_set(Token name, Variable member);
Object construct_object(GeneralTypeToken context);

bool has_method(Object obj, Token name);
bool has_member(Object obj, Token name);

Function* get_method(Object* obj, Token name);
Variable* get_member(Object* obj, Token name);

Variable run_method(Object& obj, Token name, std::vector<Variable> args);

MEOWSCRIPT_HEADER_END

#endif