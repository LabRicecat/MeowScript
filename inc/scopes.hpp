#ifndef MEOWSCRIPT_IG_SCOPES_HPP
#define MEOWSCRIPT_IG_SCOPES_HPP

#include "defs.hpp"
#include "reader.hpp"
#include "variables.hpp"
#include "functions.hpp"

#include <map>
#include <stack>

MEOWSCRIPT_HEADER_BEGIN

struct Scope {
    std::map<std::string,Variable> vars;
    std::map<std::string,Function> functions;
    int parent = -1;
    unsigned int index;
    bool freed = false;
    bool last_if_result = true;

    unsigned int current_line = 1;
};

inline std::vector<Scope> scopes;
inline std::stack<int> scope_trace;

Scope* current_scope();

// parent = -1 => last
// parent = -2 => none
void new_scope(int parent = -1, std::map<std::string,Variable> external_vars = {});
void pop_scope(bool save = false);

// The external_vars NOT be deleted, but overriden if they already exist!
void load_scope(int idx, std::map<std::string,Variable> external_vars = {});

unsigned int get_new_scope();

bool is_variable(std::string name);
Variable* get_variable(std::string name);
void set_variable(std::string name, Variable var);
void new_variable(std::string name, Variable var);

bool is_function(std::string name);

Function* get_function(std::string name);
// returns false if it fails
// does not override existing functions in scope
bool add_function(std::string name, Function fun);

MEOWSCRIPT_HEADER_END

#endif