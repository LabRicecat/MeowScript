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
struct Parameter;

struct Object {
    int parent_scope = 0;

    std::map<std::string,Variable> members;
    std::map<std::string,std::vector<Function>> methods;
    std::map<std::string,Object> structs;

    std::vector<std::tuple<std::string,std::vector<Variable>>> on_deconstruct;
};

Function generate_get(Token name, Variable member);
Function generate_set(Token name, Variable member);
Object construct_object(GeneralTypeToken context);

bool has_method(Object obj, Token name);
bool has_member(Object obj, Token name);
bool has_struct(Object obj, Token name);

Function* get_method(Object* obj, Token name, std::vector<Variable> params);
Function* get_method(Object* obj, Token name, std::vector<Parameter> params);
Variable* get_member(Object* obj, Token name);
Object* get_struct(Object* obj, Token name);

Variable run_method(Object* obj, Token name, std::vector<Variable> args);

// `obj2` might match `obj1` but not vice versa!
// This is because we check if `obj2` has all requirements to be a
//  `obj1`
bool struct_matches(Object* obj1, Object* obj2);

bool sibling_method(std::string name);

MEOWSCRIPT_HEADER_END

#endif