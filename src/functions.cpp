#include "../inc/functions.hpp"
#include "../inc/global.hpp"
#include "../inc/runner.hpp"

MEOWSCRIPT_SOURCE_FILE

Variable MeowScript::Function::run(std::vector<Variable> args) {
    if(args.size() != this->args.size()) {
        std::string err = "Too many/few arguemnts for function!\n\t- Expected: " + std::to_string(this->args.size()) + "\n\t- But got: " + std::to_string(args.size());
        throw errors::MWSMessageException{err,global::get_line()};
    }
    
    std::map<std::string,Variable> arg_map;
    for(size_t i = 0; i < args.size(); ++i) {
        if(this->args[i] != Variable::Type::UNKNOWN && this->args[i] != Variable::Type::ANY && args[i].type != this->args[i]) {
            std::string err = "Invalid argument! (" + std::to_string(i) + ")\n\t- Expected: " + var_t2token(this->args[i]).content + "\n\t- But got: " + var_t2token(args[i].type).content;
            throw errors::MWSMessageException{err,global::get_line()};
        }

        arg_map[arg_names[i]] = args[i]; 
    }
    int saved_istruct = global::in_struct;
    global::in_struct = 0;
    GeneralTypeToken gtt_ret = run_lexed(body,true,true,scope_idx,arg_map,this->file);
    global::in_struct = saved_istruct;
    if(gtt_ret.type != General_type::VOID && gtt_ret.type != General_type::UNKNOWN) {
        Variable var_ret;
        try {
            var_ret = gtt_ret.to_variable();
        }
        catch(errors::MWSMessageException& err) {
            std::string err_msg = "Invalid return type!\n\t- Expected: " + var_t2token(return_type).content + "\n\t- But got: " + general_t2token(gtt_ret.type).content;
            throw errors::MWSMessageException{err_msg,global::get_line()};
        }

        if(var_ret.type != return_type && return_type != Variable::Type::ANY) {
            std::string err_msg = "Invalid return type!\n\t- Expected: " + var_t2token(return_type).content + "\n\t- But got: " + var_t2token(var_ret.type).content;
            throw errors::MWSMessageException{err_msg,global::get_line()};
        }
        return var_ret;
    }
    else {
        return Variable();
    }
}