#ifndef MEOWSCRIPT_IG_OBJECTS_HPP
#define MEOWSCRIPT_IG_OBJECTS_HPP

#include "defs.hpp"
#include "functions.hpp"
#include "variables.hpp"

MEOWSCRIPT_HEADER_BEGIN

struct Scope;
using Object = Scope*;

Function generate_get(Token name, Variable member);
Function generate_set(Token name, Variable member);
Object construct_object(GeneralTypeToken context);

bool has_method(Object obj, Token name);
bool has_member(Object obj, Token name);

Function* get_method(Object obj, Token name);
Variable* get_member(Object obj, Token name);

Variable run_method(Object, Token name, std::vector<Variable> args);

MEOWSCRIPT_HEADER_END

#endif