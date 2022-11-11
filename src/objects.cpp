#include "../inc/objects.hpp"
#include "../inc/scopes.hpp"
#include "../inc/runner.hpp"
#include "../inc/scopes.hpp"

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
    new_scope(0);
    ++global::in_struct;
    run_text(context.source.content,false,true,-1,{},"",false,true);
    --global::in_struct;
    retobj = current_scope();
    pop_scope(true);
    return retobj;
}

bool MeowScript::has_method(Object obj, Token name) {
    return obj->functions.count(name.content) != 0;
}
bool MeowScript::has_member(Object obj, Token name) {
    return obj->vars.count(name.content) != 0;
}

Function* MeowScript::get_method(Object obj, Token name) {
    if(!has_method(obj,name)) {
        return nullptr;
    }
    return &obj->functions[name];
}
Variable* MeowScript::get_member(Object obj, Token name) {
    if(!has_member(obj,name)) {
        return nullptr;
    }
    return &obj->vars[name];
}

Variable MeowScript::run_method(Object obj, Token name, std::vector<Variable> args) {
    Function* func;
    int index = obj->index;
    while(index != -1) {
        for(auto& i : scopes[index].functions) {
            if(i.first == name.content) {
                func = &i.second;
            }
        }
        index = scopes[index].parent;
    }
    if(func == nullptr) {
        return Variable();
    }
    load_scope(obj->index);
    Variable ret = func->run(args);
    pop_scope(true);
    return ret;
}