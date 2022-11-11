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
};

bool is_operator(std::string name);

Operator* get_operator(std::string name, General_type left, General_type right);

Variable parse_expression(std::string s);

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

        {"=", { // assign
            {
                General_type::NAME, General_type::UNKNOWN, -999,
                [](GeneralTypeToken left, GeneralTypeToken right)->Variable {
                    if(!is_variable(left.source)) {
                        throw errors::MWSMessageException{"Can't assign to not existing variable!",global::get_line()};
                    }
                    set_variable(left.source,right.to_variable());
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