#include "../inc/scopes.hpp"
#include "../inc/global.hpp"

MEOWSCRIPT_SOURCE_FILE

Scope* MeowScript::current_scope() {
    if(scope_trace.empty()) {
        return nullptr;
    }
    return &scopes[scope_trace.top()];
}

void MeowScript::new_scope(int parent, std::map<std::string,Variable> external_vars) {
    Scope ns;
    if(parent != -2) {
        if(parent == -1 && scopes.size() != 0) {
            parent = scopes[scope_trace.top()].index;
        }
        ns.parent = parent;
    }
    for(auto& i : scopes) {
        if(i.freed) {
            ns.index = i.index;
            i.vars.clear();
            i.freed = false;
            i.parent = ns.parent;
            for(auto j : external_vars) {
                i.vars[j.first] = j.second;
            }
            scope_trace.push(i.index);
            return;
        }
    }
    for(auto i : external_vars) {
        ns.vars[i.first] = i.second;
    }
    ns.index = scopes.size();
    scope_trace.push(ns.index);
    scopes.push_back(ns);
}

void MeowScript::pop_scope(bool save) {
    if(!scope_trace.empty()) {
        for(auto& i : current_scope()->vars) {
            if(i.second.type == Variable::Type::Object) {
                call_obj_deconstruct(i.second.storage.obj);
            }
        }
        if(!save) {
            scopes[current_scope()->index].freed = true;
            scopes[current_scope()->index].current_obj = {};
            scopes[current_scope()->index].functions = {};
            scopes[current_scope()->index].vars = {};
            scopes[current_scope()->index].structs = {};
        }

        scope_trace.pop();
    }
}

void MeowScript::load_scope(int idx, std::map<std::string,Variable> external_vars, bool hard_copy) {
    if(hard_copy) {
        scope_trace.push(idx);
        for(auto i : external_vars) {
            current_scope()->vars[i.first] = i.second;
        }
    }
    else {
        new_scope(scopes[idx].parent,scopes[idx].vars);
        current_scope()->functions = scopes[idx].functions;
        for(auto i : external_vars) {
            current_scope()->vars[i.first] = i.second;
        }
    }
}

void MeowScript::call_obj_deconstruct(Object& obj) {
    for(auto j : obj.on_deconstruct) {
        get_method(&obj,j)->run({});
    }
}

unsigned int MeowScript::get_new_scope() {
    for(size_t i = 0; i < scopes.size(); ++i) {
        if(scopes[i].freed) {
            scopes[i].freed = false;
            scopes[i].vars = {};
            scopes[i].functions = {};
            scopes[i].structs = {};
            scopes[i].parent = 0;
            scopes[i].current_obj = {};
            return i;
        }
    }
    scopes.push_back(Scope());
    scopes.back().index = scopes.size()-1;
    return scopes.size()-1;
}

bool MeowScript::is_variable(std::string name) {
    return get_variable(name) != nullptr;
}

Variable* MeowScript::get_variable(std::string name) {
    int index = current_scope()->index;
    while(index > -1) {
        for(auto& i : scopes[index].vars) {
            if(i.first == name) {
                return &i.second;
            }
        }
        index = scopes[index].parent;
    }
    return nullptr;
}

void MeowScript::set_variable(std::string name, Variable var) {
    Variable* vptr = get_variable(name);
    if(vptr == nullptr) {
        new_variable(name,var);
        return;
    }
    if(vptr->constant) {
        throw errors::MWSMessageException{"Const variable \"" + name + "\" can not get a new value! (" + var.to_string() + ")",global::get_line()};
    }
    //if(vptr->type == Variable::Type::Object) {
        //call_obj_deconstruct(vptr->storage.obj);
    //}
    vptr->set(var); // TODO: on error -> handling
}

void MeowScript::new_variable(std::string name, Variable var) {
    current_scope()->vars[name] = var;
}

bool MeowScript::is_function(std::string name) {
    return get_function(name) != nullptr;
}

bool MeowScript::is_event(std::string name) {
    return global::events.count(name) != 0;
}

bool MeowScript::is_object(std::string name) {
    int index = current_scope()->index;
    while(index > -1) {
        for(auto i : scopes[index].vars) {
            if(i.first == name && i.second.type == Variable::Type::Object) {
                return true;
            }
        }
        index = scopes[index].parent;
    }
    return false;
}

bool MeowScript::is_struct(std::string name) {
    int index = current_scope()->index;
    while(index > -1) {
        for(auto i : scopes[index].structs) {
            if(i.first == name) {
                return true;
            }
        }
        index = scopes[index].parent;
    }
    return false;
}

Object* MeowScript::get_struct(std::string name) {
    int index = current_scope()->index;
    while(index > -1) {
        for(auto& i : scopes[index].structs) {
            if(i.first == name) {
                return &i.second;
            }
        }
        index = scopes[index].parent;
    }
    return nullptr;
}

Object* MeowScript::get_object(std::string name) {
    int index = current_scope()->index;
    while(index > -1) {
        for(auto& i : scopes[index].vars) {
            if(i.first == name && i.second.type == Variable::Type::Object) {
                return &i.second.storage.obj;
            }
        }
        index = scopes[index].parent;
    }
    return nullptr;
}

Function* MeowScript::get_function(std::string name) {
    int index = current_scope()->index;
    while(index > -1) {
        for(auto& i : scopes[index].functions) {
            if(i.first == name) {
                return &i.second;
            }
        }
        index = scopes[index].parent;
    }
    return nullptr;
}

bool MeowScript::add_function(std::string name, Function fun) {
    for(auto i : current_scope()->functions) {
        if(i.first == name) {
            return false;
        }
    }
    if(fun.scope_idx == 0) {
        fun.scope_idx = get_new_scope();
        scopes[fun.scope_idx].parent = current_scope()->index;
    }
    current_scope()->functions[name] = fun;
    return true;
}

void MeowScript::add_object(std::string name, Object obj) {
    for(auto& i : obj.methods) {
        int sscope = i.second.scope_idx;
        i.second.scope_idx = get_new_scope();
        scopes[i.second.scope_idx] = scopes[sscope];
        scopes[i.second.scope_idx].parent = obj.parent_scope;
        scopes[i.second.scope_idx].index = i.second.scope_idx;
    }
    set_variable(name,obj);
}

bool MeowScript::add_struct(std::string name, Object struc) {
    for(auto i : current_scope()->structs) {
        if(i.first == name) {
            return false;
        }
    }
    current_scope()->structs[name] = struc;
    return true;
}