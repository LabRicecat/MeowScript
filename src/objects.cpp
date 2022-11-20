#include "../inc/objects.hpp"
#include "../inc/scopes.hpp"
#include "../inc/runner.hpp"
#include "../inc/scopes.hpp"
#include "../inc/variables.hpp"
#include "../inc/functions.hpp"

MEOWSCRIPT_SOURCE_FILE

bool MeowScript::struct_matches(Object* obj1, Object* obj2) {
    if(obj1->members.size() > obj2->members.size() || obj1->methods.size() > obj2->methods.size() || obj1->structs.size() > obj2->structs.size()) {
        return false;
    }
    for(auto i : obj1->members) {
        Variable* vr_obj2 = get_member(obj2,i.first);
        if(vr_obj2 == nullptr || (i.second.fixed_type && vr_obj2->type != i.second.type)) {
            return false;
        }
    }
    for(auto i : obj1->methods) {
        for(auto j : i.second) {
            Function* fn_obj2 = get_method(obj2,i.first,j.params);
            if(fn_obj2 == nullptr || !j.return_type.matches(fn_obj2->return_type) || !paramlist_matches(j.params,fn_obj2->params)) {
                return false;
            }
        }
    }
    for(auto i : obj1->structs) {
        Object* st_obj2 = get_struct(obj2,i.first);
        if(st_obj2 == nullptr || !struct_matches(&i.second,st_obj2)) {
            return false;
        }
    }
    return true;
}

Function MeowScript::generate_get(Token name, Variable member) {
    Function func;
    func.body = lex_text("return " + name.content + "\n");
    if(member.fixed_type)
        func.return_type = member.type;
    else
        func.return_type = Variable::Type::ANY;
    
    return func;
}

Function MeowScript::generate_set(Token name, Variable member) {
    Function func;
    func.body = lex_text("set " + name.content + " " + name.content +"_set\n");
    if(member.fixed_type)
        func.params.push_back(Parameter(member.type,name.content +"_set"));
    else
        func.params.push_back(Parameter(Variable::Type::ANY,name.content +"_set"));
    func.return_type = Variable::Type::VOID;
    return func;
}

Object MeowScript::construct_object(GeneralTypeToken context) {
    if(context.type != General_type::COMPOUND) {
        return Object{};
    }
    context.source.content.erase(context.source.content.begin());
    context.source.content.erase(context.source.content.end()-1);
    Object retobj;
    new_scope(-1);
    ++global::in_struct;
    current_scope()->current_obj.push(&retobj);
    run_text(context.source.content,false,true,-1,{},"",false,true);
    current_scope()->current_obj.pop();
    --global::in_struct;
    retobj.members = current_scope()->vars;
    retobj.methods = current_scope()->functions;
    retobj.structs = current_scope()->structs;
    retobj.parent_scope = current_scope()->parent;

    scopes[current_scope()->index].freed = true;
    scopes[current_scope()->index].current_obj = {};
    scopes[current_scope()->index].functions = {};
    scopes[current_scope()->index].vars = {};
    scopes[current_scope()->index].structs = {};
    
    scope_trace.pop();
    return retobj;
}

bool MeowScript::has_method(Object obj, Token name) {
    return obj.methods.count(name.content) != 0;
}
bool MeowScript::has_member(Object obj, Token name) {
    return obj.members.count(name.content) != 0;
}
bool MeowScript::has_struct(Object obj, Token name) {
    return obj.structs.count(name.content) != 0;
}


Function* MeowScript::get_method(Object* obj, Token name, std::vector<Variable> params) {
    if(!has_method(*obj,name)) {
        return nullptr;
    }
    for(auto& i : obj->methods[name]) {
        if(func_param_match(i,params)) {
            return &i;
        }
    }
    return nullptr;
}
Function* MeowScript::get_method(Object* obj, Token name, std::vector<Parameter> params) {
    if(!has_method(*obj,name)) {
        return nullptr;
    }
    for(auto& i : obj->methods[name]) {
        if(func_param_match(i,params)) {
            return &i;
        }
    }
    return nullptr;
}
Variable* MeowScript::get_member(Object* obj, Token name) {
    if(!has_member(*obj,name)) {
        return nullptr;
    }
    return &obj->members[name.content];
}
Object* MeowScript::get_struct(Object* obj, Token name) {
    if(!has_struct(*obj,name)) {
        return nullptr;
    }
    return &obj->structs[name.content];
}

Variable MeowScript::run_method(Object* obj, Token name, std::vector<Variable> args) {
    Function* func = get_method(obj,name.content,args);
    if(func == nullptr) {
        return Variable();
    }
    new_scope(obj->parent_scope);

    current_scope()->vars = obj->members;
    current_scope()->functions = obj->methods;
    current_scope()->structs = obj->structs;

    Variable ret = func->run(args,true);

    obj->members = current_scope()->vars;
    obj->methods = current_scope()->functions;
    obj->structs = current_scope()->structs;

    pop_scope(false);

    return ret;
}