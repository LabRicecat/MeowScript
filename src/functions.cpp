#include "../inc/functions.hpp"
#include "../inc/global.hpp"
#include "../inc/runner.hpp"
#include "../inc/objects.hpp"

MEOWSCRIPT_SOURCE_FILE

bool Parameter::matches(Variable var) const {
    return \
        type == Variable::Type::UNKNOWN ||
        type == Variable::Type::ANY ||
        (
            var.type == Variable::Type::Object &&
            struct_name == "" &&
            type == Variable::Type::Object
        ) ||
        (
            var.type == Variable::Type::Object &&
            type == Variable::Type::Object &&
            struct_matches(get_struct(struct_name),&var.storage.obj)
        ) ||
        (
            var.type != Variable::Type::Object &&
            var.type == type
        );
}
// TODO: fix potential recursion problem
bool Parameter::matches(Parameter param) const {
    return \
        type == Variable::Type::UNKNOWN ||
        type == Variable::Type::ANY ||
        (
            param.type == Variable::Type::Object &&
            struct_name == "" &&
            type == Variable::Type::Object
        ) ||
        (
            param.type == Variable::Type::Object &&
            param.struct_name == "" &&
            type == Variable::Type::Object
        ) ||
        (
            param.type == Variable::Type::Object &&
            type == Variable::Type::Object &&
            struct_matches(get_struct(struct_name),get_struct(param.struct_name))
        ) ||
        (
            param.type != Variable::Type::Object &&
            param.type == type
        );
}

bool MeowScript::paramlist_matches(std::vector<Parameter> params1,std::vector<Parameter> params2) {
    for(size_t i = 0; i < params1.size(); ++i) {
        if(!params1[i].matches(params2[i])) {
            return false;
        }
    }
    return true;
}


Variable MeowScript::Function::run(std::vector<Variable> args, bool method_mode) {
    if(args.size() != this->params.size()) {
        std::string err = "Too many/few arguemnts for function!\n\t- Expected: " + std::to_string(this->params.size()) + "\n\t- But got: " + std::to_string(args.size());
        throw errors::MWSMessageException{err,global::get_line()};
    }
    
    std::map<std::string,Variable> arg_map;
    for(size_t i = 0; i < args.size(); ++i) {
        if(!this->params[i].matches(args[i])) {
            std::string err = "Invalid argument! (" + std::to_string(i) + ")\n\t- Expected: " + var_t2token(this->params[i].type).content + "\n\t- But got: " + var_t2token(args[i].type).content;
            throw errors::MWSMessageException{err,global::get_line()};
        }

        arg_map[params[i].name] = args[i]; 
    }
    int saved_istruct = global::in_struct;
    global::in_struct = 0;
    GeneralTypeToken gtt_ret;
    if(method_mode) {
        gtt_ret = run_lexed(body,false,false,-1,arg_map,this->file);
    }
    else {
        gtt_ret = run_lexed(body,false,true,scope_idx,arg_map,this->file);
    }
    global::in_struct = saved_istruct;
    if(gtt_ret.type != General_type::VOID && gtt_ret.type != General_type::UNKNOWN) {
        Variable var_ret;
        try {
            var_ret = gtt_ret.to_variable();
        }
        catch(errors::MWSMessageException& err) {
            std::string err_msg = "Invalid return type!\n\t- Expected: " + var_t2token(return_type.type).content + "\n\t- But got: " + general_t2token(gtt_ret.type).content;
            throw errors::MWSMessageException{err_msg,global::get_line()};
        }
        return var_ret;
    }
    else {
        return Variable();
    }
}