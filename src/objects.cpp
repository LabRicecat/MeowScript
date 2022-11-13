#include "../inc/objects.hpp"
#include "../inc/scopes.hpp"
#include "../inc/runner.hpp"
#include "../inc/scopes.hpp"
#include "../inc/variables.hpp"
#include "../inc/functions.hpp"

MEOWSCRIPT_SOURCE_FILE

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
        func.args.push_back(member.type);
    else
        func.args.push_back(Variable::Type::ANY);
    func.arg_names.push_back(name.content +"_set");
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
    pop_scope(false);
    return retobj;
}

bool MeowScript::has_method(Object obj, Token name) {
    return obj.methods.count(name.content) != 0;
}
bool MeowScript::has_member(Object obj, Token name) {
    return obj.members.count(name.content) != 0;
}

Function* MeowScript::get_method(Object* obj, Token name) {
    if(!has_method(*obj,name)) {
        return nullptr;
    }
    return &obj->methods[name];
}
Variable* MeowScript::get_member(Object* obj, Token name) {
    if(!has_member(*obj,name)) {
        return nullptr;
    }
    return &obj->members[name.content];
}

Variable MeowScript::run_method(Object& obj, Token name, std::vector<Variable> args) {
    Function* func = get_method(&obj,name.content);
    if(func == nullptr) {
        return Variable();
    }

    new_scope(obj.parent_scope);

    current_scope()->vars = obj.members;
    current_scope()->functions = obj.methods;
    current_scope()->structs = obj.structs;

    Variable ret = func->run(args);

    obj.members = current_scope()->vars;
    obj.methods = current_scope()->functions;
    obj.structs = current_scope()->structs;

    pop_scope(false);

    return ret;
}