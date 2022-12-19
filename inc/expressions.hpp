#ifndef MEOWSCRIPT_IG_EXPRESSIONS_HPP
#define MEOWSCRIPT_IG_EXPRESSIONS_HPP

#include "defs.hpp"
#include "variables.hpp"
#include "scopes.hpp"
#include "global.hpp"

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
        {"&", { // xor
            {
                General_type::NUMBER, General_type::NUMBER, -1,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    Variable left_v = left.to_variable();
                    Variable right_v = right.to_variable();
                    return (long)left_v.storage.number & (long)right_v.storage.number;
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
    }
};

MEOWSCRIPT_HEADER_END

#endif