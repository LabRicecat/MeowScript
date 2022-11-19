#ifndef MEOWSCRIPT_IG_SCOPES_HPP
#define MEOWSCRIPT_IG_SCOPES_HPP

#include "defs.hpp"
#include "reader.hpp"
#include "variables.hpp"
#include "functions.hpp"
#include "objects.hpp"

#include <map>
#include <stack>

MEOWSCRIPT_HEADER_BEGIN

struct Object;

struct Scope {
    std::map<std::string,Variable> vars;
    std::map<std::string,Function> functions;
    std::map<std::string,Object> structs;
    int parent = -1;
    int index = -1;
    bool freed = false;
    bool last_if_result = true;

    unsigned int current_line = 1;

    std::stack<Object*> current_obj;
};

inline std::vector<Scope> scopes;
inline std::stack<int> scope_trace;

Scope* current_scope();

// parent = -1 => last
// parent = -2 => none
void new_scope(int parent = -1, std::map<std::string,Variable> external_vars = {});
void pop_scope(bool save = false);

// The external_vars are NOT being deleted, but overriden if they already exist!
// Actually just copies the data of the scope
void load_scope(int idx, std::map<std::string,Variable> external_vars = {}, bool hard_load = false);

void call_obj_deconstruct(Object& obj);

unsigned int get_new_scope();

bool is_variable(std::string name);
Variable* get_variable(std::string name);
void set_variable(std::string name, Variable var);
void new_variable(std::string name, Variable var);

bool is_function(std::string name);
bool is_object(std::string name);
bool is_struct(std::string name);

Object* get_struct(std::string name);
Object* get_object(std::string name);

Function* get_function(std::string name);
// returns false if it fails
// does not override existing functions in scope
bool add_function(std::string name, Function fun);
void add_object(std::string name, Object obj);
bool add_struct(std::string name, Object struc);

MEOWSCRIPT_HEADER_END

#endif