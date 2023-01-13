#ifndef MEOWSCRIPT_IG_EXPRESSIONS_HPP
#define MEOWSCRIPT_IG_EXPRESSIONS_HPP

#include "defs.hpp"
#include "variables.hpp"
#include "scopes.hpp"
#include "global.hpp"

#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <math.h>

MEOWSCRIPT_HEADER_BEGIN

// RULE: all operators with the SAME name MUST have the SAME priority!
// If not, the first priority is taken
struct Operator {
    General_type req_left;
    General_type req_right;
    int priority = 0;
    
    Variable (*parse)(GeneralTypeToken left, GeneralTypeToken right);

    bool operator==(Operator op) {
        return req_left == op.req_left &&
                req_right == op.req_right &&
                priority == priority;
    }
};

bool is_operator(std::string name);

Operator* get_operator(std::string name, General_type left, General_type right);

Variable parse_expression(std::string s);

bool is_expression(std::string s);

inline std::set<std::string> operator_names = {
    "+","-","*","/","&&","&","|","||",".","..","..=",">",
    "<",">=","<=","=>","->","==","!=","=","%",":","::","!"
};

inline std::unordered_map<std::string,std::vector<Operator>> operators = {
    {
        {"+", {
            {
                General_type::NUMBER, General_type::NUMBER, 1,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return left_v.storage.number + right_v.storage.number;
                }
            },
            {
                General_type::STRING, General_type::STRING, 1,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    Variable ret;
                    ret.set(left_v.storage.string.content + right_v.storage.string.content);
                    return ret;
                }
            },
        }},
        {"-", {
            {
                General_type::NUMBER, General_type::NUMBER, 1,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return left_v.storage.number - right_v.storage.number;
                }
            },
        }},
        {"*", {
            {
                General_type::NUMBER, General_type::NUMBER, 2,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return left_v.storage.number * right_v.storage.number;
                }
            },
        }},
        {"/", {
            {
                General_type::NUMBER, General_type::NUMBER, 2,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    if(right_v.storage.number == 0) {
                        throw errors::MWSMessageException{"Can't devide by 0!",global::get_line()};
                    }
                    return left_v.storage.number / right_v.storage.number;
                }
            },
        }},
        {"%", {
            {
                General_type::NUMBER, General_type::NUMBER, 2,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    if(right_v.storage.number == 0) {
                        throw errors::MWSMessageException{"Can't devide by 0!",global::get_line()};
                    }
                    return int(left_v.storage.number) % int(right_v.storage.number);
                }
            },
        }},
        {"^", {
            {
                General_type::NUMBER, General_type::NUMBER, 3,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return std::pow(left_v.storage.number,right_v.storage.number);
                }
            },
        }},
        {"==", {
            {
                General_type::NUMBER, General_type::NUMBER, 0,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return left_v.storage.number == right_v.storage.number;
                }
            },
            {
                General_type::STRING, General_type::STRING, 0,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return left_v.storage.string.content == right_v.storage.string.content;
                }
            },
            {
                General_type::OBJECT, General_type::OBJECT, 0,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return struct_matches(&left_v.storage.obj,&right_v.storage.obj);
                }
            },
        }},
        {"!=", {
            {
                General_type::NUMBER, General_type::NUMBER, 0,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return left_v.storage.number != right_v.storage.number;
                }
            },
            {
                General_type::STRING, General_type::STRING, 0,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return left_v.storage.string.content != right_v.storage.string.content;
                }
            },
            {
                General_type::OBJECT, General_type::OBJECT, 0,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return !struct_matches(&left_v.storage.obj,&right_v.storage.obj);
                }
            },
        }},
        {">", {
            {
                General_type::NUMBER, General_type::NUMBER, 0,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return left_v.storage.number > right_v.storage.number;
                }
            },
        }},
        {"<", {
            {
                General_type::NUMBER, General_type::NUMBER, 0,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return left_v.storage.number < right_v.storage.number;
                }
            },
        }},
        {"<=", {
            {
                General_type::NUMBER, General_type::NUMBER, 0,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return left_v.storage.number <= right_v.storage.number;
                }
            },
        }},
        {">=", {
            {
                General_type::NUMBER, General_type::NUMBER, 0,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return left_v.storage.number >= right_v.storage.number;
                }
            },
        }},
        {"&&", {
            {
                General_type::NUMBER, General_type::NUMBER, -1,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return left_v.storage.number==1 && right_v.storage.number==1;
                }
            },
        }},
        {"||", {
            {
                General_type::NUMBER, General_type::NUMBER, -1,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return left_v.storage.number==1 || right_v.storage.number==1;
                }
            },
        }},
        {"^^", { // xor
            {
                General_type::NUMBER, General_type::NUMBER, -1,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return left_v.storage.number==1 ^ right_v.storage.number==1;
                }
            },
        }},
        {"|", {
            {
                General_type::NUMBER, General_type::NUMBER, -1,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return (long)left_v.storage.number | (long)right_v.storage.number;
                }
            },
        }},
        {"&", {
            {
                General_type::NUMBER, General_type::NUMBER, -1,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return (long)left_v.storage.number & (long)right_v.storage.number;
                }
            },
        }},
        {"..", {
            {
                General_type::NUMBER, General_type::NUMBER, 4,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    List lst;
                    for(int i = left_v.storage.number; i < right_v.storage.number; ++i) {
                        lst.elements.push_back(i);
                    }
                    return lst;
                }
            },
        }},
        {"..=", {
            {
                General_type::NUMBER, General_type::NUMBER, 4,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    List lst;
                    for(int i = left_v.storage.number; i <= right_v.storage.number; ++i) {
                        lst.elements.push_back(i);
                    }
                    return lst;
                }
            },
        }},

        {"=", { // assign
            {
                General_type::NAME, General_type::UNKNOWN, -999,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    if(!is_variable(left.source)) {
                        throw errors::MWSMessageException{"Can't assign to not existing variable!",global::get_line()};
                    }
                    Variable s;
                    GeneralTypeToken gtt;
                    bool rethrow = false;
                    try {
                        switch(right.type) {
                            case General_type::NAME:
                                s = tools::check4var(right).to_variable();
                                break;
                            case General_type::FUNCTION:
                                s = right.to_variable();
                                break;
                            case General_type::COMPOUND:
                                try {
                                    gtt = tools::check4compound(right);
                                }
                                catch(errors::MWSMessageException& err) {
                                    rethrow = true;
                                    throw err;
                                }
                                s = gtt.to_variable();
                                break;
                            case General_type::EXPRESSION:
                                s = tools::check4expression(right).to_variable();
                                break;
                            case General_type::FUNCCALL:
                                while(right.type == General_type::FUNCCALL) {
                                    right = evaluate_func_call(right.funccall);
                                }
                            default:
                                s = right.to_variable();
                        }
                    }
                    catch(errors::MWSMessageException& err) {
                        if(rethrow) {
                            throw err;
                        }
                        throw errors::MWSMessageException{"Invalid assign to variable! Only VariableType's allowed, but got: " + general_t2token(right.type).content,global::get_line()};
                    }
                    set_variable(left.source,s);
                    Variable ret;
                    ret.type = Variable::Type::VOID;
                    return ret;
                }
            },
        }},
    
        {".", {
            {
                General_type::NAME, General_type::FUNCCALL, 999,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    //Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();

                    if(!is_variable(left.to_string())) {
                        // TODO: add error message
                    }
                    else {
                        Variable call = *get_variable(left.to_string());
                        std::string name = right_v.storage.function_call.func;
                        argument_list arglist = right_v.storage.function_call.arglist;

                        if(call.type == Variable::Type::Object) {
                            Object obj = *get_object(left.to_string());
                            Token method_name = right_v.storage.function_call.func;

                            if(!has_method(obj,method_name)) {
                                throw errors::MWSMessageException{"Unknown object method: " + method_name.content,global::get_line()};
                            }

                            std::vector<Variable> args;
                            std::vector<GeneralTypeToken> arglist = right_v.storage.function_call.arglist;

                            for(auto i : arglist) {
                                args.push_back(tools::check4placeholder(i).to_variable());
                            }

                            Function* method = get_method(&obj,method_name,args);
                            if(method == nullptr) {
                                std::string err_msg = "No overload of method " + method_name.content + " matches agumentlist!\n- Got: [";
                                for(auto i : args) {
                                    err_msg += var_t2token(i.type).content + ",";
                                }
                                err_msg.pop_back();
                                throw errors::MWSMessageException{err_msg + "]",global::get_line()};
                            }

                            Variable ret = run_method(&obj,method_name,args);
                            *get_object(left.to_string()) = obj;
                            if(right_v.storage.function_call.shadow_return) return general_null;
                            return ret;
                        }
                        else if(call.type == Variable::Type::String) {
                            Method<Token>* method = get_string_method(name);
                            if(method == nullptr) {
                                throw errors::MWSMessageException{"Unknown string method: " + name,global::get_line()};
                            }

                            std::vector<Variable> args;
                            
                            if(method->args.size() != arglist.size()) {
                                std::string err = "Too many/few arguments for string method: " + method->name + "\n\t- Expected: " + std::to_string(method->args.size()) + "\n\t- But got: " + std::to_string(arglist.size());
                                throw errors::MWSMessageException{err,global::get_line()};
                            }

                            for(size_t j = 0; j < method->args.size(); ++j) {
                                while(arglist[j].type == General_type::FUNCCALL) {
                                    arglist[j] = evaluate_func_call(arglist[j].funccall);
                                }
                                auto identf = arglist[j].type;
                                if(!method->args[j].matches(identf)) {
                                    std::string err_msg = "Invalid argument:\n\t- Expected: [";
                                    for(size_t k = 0; k < method->args[j].carry.size(); ++k) {
                                        err_msg += (k == 0 ? "":",") + general_t2token(General_type(method->args[j].carry[k]-1)).content;
                                    }
                                    err_msg += "]\n\t- But got: " + general_t2token(arglist[j].type).content + " (" + arglist[j].to_string() + ")";
                                    throw errors::MWSMessageException(err_msg,global::get_line());
                                }
                                else {
                                    Variable v = arglist[j].to_variable();
                                    args.push_back(v);
                                }
                            }

                            Token tk = get_variable(left.source)->storage.string;
                            Variable ret = method->run(args,&tk);
                            get_variable(left.source)->storage.string = tk;
                            return ret;
                        }
                    }
                    return general_null;
                }
            },
            {
                General_type::LIST, General_type::STRING, 999,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    Variable ret;
                    ret.set(left_v.storage.string.content + right_v.storage.string.content);
                    return ret;
                }
            },
        }},
    }
};

MEOWSCRIPT_HEADER_END

#endif