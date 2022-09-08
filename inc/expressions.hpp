#ifndef MEOWSCRIPT_IG_EXPRESSIONS_HPP
#define MEOWSCRIPT_IG_EXPRESSIONS_HPP

#include "defs.hpp"
#include "variables.hpp"
#include "global.hpp"

#include <stack>
#include <string>
#include <unordered_map>
#include <math.h>

MEOWSCRIPT_HEADER_BEGIN

// RULE: all operators with the SAME name MUST have the SAME priority!
// If not, the first priority is taken
struct Operator {
    Variable::Type req_left;
    Variable::Type req_right;
    int priority = 0;
    
    Variable (*parse)(Variable left, Variable right);
};

bool is_operator(std::string name);

Operator* get_operator(std::string name, Variable::Type left, Variable::Type right);

Variable parse_expression(std::string s);

inline std::unordered_map<std::string,std::vector<Operator>> operators = {
    {
        {"+", {
            {
                Variable::Type::Number, Variable::Type::Number, 1,
                [](Variable left, Variable right)->Variable {
                    return left.storage.number + right.storage.number;
                }
            },
            {
                Variable::Type::String, Variable::Type::String, 1,
                [](Variable left, Variable right)->Variable {
                    Variable ret;
                    ret.set(left.storage.string.content + right.storage.string.content);
                    return ret;
                }
            },
        }},
        {"-", {
            {
                Variable::Type::Number, Variable::Type::Number, 1,
                [](Variable left, Variable right)->Variable {
                    return left.storage.number - right.storage.number;
                }
            },
        }},
        {"*", {
            {
                Variable::Type::Number, Variable::Type::Number, 2,
                [](Variable left, Variable right)->Variable {
                    return left.storage.number * right.storage.number;
                }
            },
        }},
        {"/", {
            {
                Variable::Type::Number, Variable::Type::Number, 2,
                [](Variable left, Variable right)->Variable {
                    if(right.storage.number == 0) {
                        throw errors::MWSMessageException{"Can't devide by 0!",global::get_line()};
                    }
                    return left.storage.number / right.storage.number;
                }
            },
        }},
        {"%", {
            {
                Variable::Type::Number, Variable::Type::Number, 2,
                [](Variable left, Variable right)->Variable {
                    if(right.storage.number == 0) {
                        throw errors::MWSMessageException{"Can't devide by 0!",global::get_line()};
                    }
                    return int(left.storage.number) % int(right.storage.number);
                }
            },
        }},
        {"^", {
            {
                Variable::Type::Number, Variable::Type::Number, 3,
                [](Variable left, Variable right)->Variable {
                    return std::pow(left.storage.number,right.storage.number);
                }
            },
        }},
        {"==", {
            {
                Variable::Type::Number, Variable::Type::Number, 0,
                [](Variable left, Variable right)->Variable {
                    return left.storage.number == right.storage.number;
                }
            },
            {
                Variable::Type::String, Variable::Type::String, 0,
                [](Variable left, Variable right)->Variable {
                    return left.storage.string.content == right.storage.string.content;
                }
            },
        }},
        {"!=", {
            {
                Variable::Type::Number, Variable::Type::Number, 0,
                [](Variable left, Variable right)->Variable {
                    return left.storage.number != right.storage.number;
                }
            },
            {
                Variable::Type::String, Variable::Type::String, 0,
                [](Variable left, Variable right)->Variable {
                    return left.storage.string.content != right.storage.string.content;
                }
            },
        }},
        {">", {
            {
                Variable::Type::Number, Variable::Type::Number, 0,
                [](Variable left, Variable right)->Variable {
                    return left.storage.number > right.storage.number;
                }
            },
        }},
        {"<", {
            {
                Variable::Type::Number, Variable::Type::Number, 0,
                [](Variable left, Variable right)->Variable {
                    return left.storage.number < right.storage.number;
                }
            },
        }},
        {"<=", {
            {
                Variable::Type::Number, Variable::Type::Number, 0,
                [](Variable left, Variable right)->Variable {
                    return left.storage.number <= right.storage.number;
                }
            },
        }},
        {">=", {
            {
                Variable::Type::Number, Variable::Type::Number, 0,
                [](Variable left, Variable right)->Variable {
                    return left.storage.number >= right.storage.number;
                }
            },
        }},
        {"&&", {
            {
                Variable::Type::Number, Variable::Type::Number, -1,
                [](Variable left, Variable right)->Variable {
                    return left.storage.number==1 && right.storage.number==1;
                }
            },
        }},
        {"||", {
            {
                Variable::Type::Number, Variable::Type::Number, -1,
                [](Variable left, Variable right)->Variable {
                    return left.storage.number==1 || right.storage.number==1;
                }
            },
        }},
        {"^^", { // xor
            {
                Variable::Type::Number, Variable::Type::Number, -1,
                [](Variable left, Variable right)->Variable {
                    return left.storage.number==1 ^ right.storage.number==1;
                }
            },
        }},
    }
};

MEOWSCRIPT_HEADER_END

#endif