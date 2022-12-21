#ifndef MEOWSCRIPT_IG_MODULES_HPP
#define MEOWSCRIPT_IG_MODULES_HPP

#include "defs.hpp"
#include <string>
#include <map>
#include <vector>

MEOWSCRIPT_HEADER_BEGIN

struct Command;
struct Variable;

struct Module {
    std::string name;
    std::vector<Command> commands;
    std::map<std::string,Variable> variables;

    Command* get_command(std::string name);
    bool has_command(std::string name);
    Variable* get_variable(std::string name);
    bool has_variable(std::string name);
    Module& add_command(Command command);
    Module& add_variable(std::string name, Variable var);

    Module() {}
    Module(std::string name) : name(name) {}
    Module(std::string name,std::vector<Command> commands) : name(name), commands(commands) {}

    bool enabled = true;

    void(*on_load)(Module* self);
};

inline std::vector<Module> modules;

Module* get_module(std::string name);
bool is_loaded_module(std::string name);

void add_module(Module module);

std::vector<Module>* get_module_list();

std::string get_username();

void make_paths();
bool check_paths();

// Unloading via deconstructor of class!
void load_all_modules();

// returns false on error
bool load_module(std::string name);

bool is_loadable_module(std::string name);

bool build_stdlib();

MEOWSCRIPT_HEADER_END

#endif