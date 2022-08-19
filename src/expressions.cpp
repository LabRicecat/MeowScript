#include "../inc/expressions.hpp"
#include "../inc/errors.hpp"
#include "../inc/global.hpp"
#include "../inc/tools.hpp"

MEOWSCRIPT_SOURCE_FILE

bool MeowScript::is_operator(std::string name) {
    return operators.count(name) != 0;
}

Operator* MeowScript::get_operator(std::string name, Variable::Type left, Variable::Type right) {
    if(!is_operator(name)) {
        return nullptr;
    }
    auto& ops = operators[name];

    for(auto& i : ops) {
        if(i.req_left == left && i.req_right == right) {
            return &i;
        }
    }
    return nullptr;
}

Variable MeowScript::parse_expression(std::string str) {
    if(!brace_check(str,'(',')')) {
        return Variable();
    }
    str.erase(str.begin());
    str.erase(str.begin()+str.size()-1);

    auto lines = lex_text(str);
    std::vector<Token> vec;
    // To make it one line
    for(auto i : lines) {
        for(auto j : i.source) {
            vec.push_back(j);
        }
    }

    std::vector<Token> lexed;
    bool look_for_opers = false;
    std::string operator_carry;
    // Try to get the operators:
    for(size_t i = 0; i < vec.size(); ++i) {
        if(vec[i].in_quotes || vec[i].content.size() != 1 || !is_valid_operator_char(vec[i].content[0]) || lexed.size() == 0) {
            lexed.push_back(vec[i]);
        }
        else {
            // Is an operator and could be merged!
            if(lexed.back().content.size() > 0 && !lexed.back().in_quotes && is_valid_operator_char(lexed.back().content[0])) {
                lexed.back().content.push_back(vec[i].content[0]);
            }
            else {
                lexed.push_back(vec[i]);
            }
        }
    }

    if(lexed.size() == 1) {
        return tools::check4placeholder(lexed[0]).to_variable();
    }

    ++global::in_expression;
    std::stack<std::string> ops;
    std::stack<Variable> st;
    for(size_t i = 0; i < lexed.size(); ++i) {
        if(get_type(lexed[i]) == General_type::EXPRESSION) {
            st.push(parse_expression(lexed[i]));
        }
        else if(!lexed[i].in_quotes && is_operator(lexed[i].content)) {
            if(st.empty()) {
                --global::in_expression;
                throw errors::MWSMessageException{"Invalid expression: " + str,global::get_line()};
            }
            auto op = operators[lexed[i].content];
            while(!ops.empty() && operators[ops.top()][0].priority >= op[0].priority) {
                Variable right = st.top(); st.pop();
                Variable left = st.top(); st.pop();

                Operator* roper = get_operator(ops.top(),left.type,right.type);
                if(roper == nullptr) {
                    --global::in_expression;
                    throw errors::MWSMessageException{"No overload of operator \"" + ops.top() + "\" matches the types: " + var_t2token(left.type).content + " | " + var_t2token(right.type).content,global::get_line()};
                }

                st.push(roper->parse(left,right));
                ops.pop();
            }
            ops.push(lexed[i].content);
        }
        else {
            st.push(tools::check4placeholder(lexed[i]).to_variable());
        }
    }


    while(!ops.empty()) {
        Variable right = st.top(); st.pop();
        Variable left = st.top(); st.pop();

        Operator* roper = get_operator(ops.top(),left.type,right.type);
        if(roper == nullptr) {
            --global::in_expression;
            throw errors::MWSMessageException{"No overload of operator \"" + ops.top() + "\" matches the types: " + var_t2token(left.type).content + " | " + var_t2token(right.type).content,global::get_line()};
        }

        st.push(roper->parse(left,right));
        ops.pop();
    }
    --global::in_expression;

    return st.top();
}