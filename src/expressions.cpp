#include "../inc/expressions.hpp"
#include "../inc/errors.hpp"
#include "../inc/global.hpp"
#include "../inc/tools.hpp"
#include "../inc/runner.hpp"

MEOWSCRIPT_SOURCE_FILE

bool MeowScript::is_operator(std::string name) {
    return operators.count(name) != 0;
}

Operator* MeowScript::get_operator(std::string name, General_type left, General_type right) {
    if(!is_operator(name)) {
        return nullptr;
    }
    auto& ops = operators[name];

    for(auto& i : ops) {
        if((i.req_left == left || i.req_left == General_type::UNKNOWN || left == General_type::UNKNOWN) && (i.req_right == right || i.req_right == General_type::UNKNOWN || right == General_type::UNKNOWN)) {
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

    if(str == "print (1+1)") {
        std::cout << "c\n";
    }

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
    bool f_op = false;
    std::stack<std::string> ops;
    std::stack<std::vector<GeneralTypeToken>> st;
    st.push({});
    for(size_t i = 0; i < lexed.size(); ++i) {
        if(!lexed[i].in_quotes && is_operator(lexed[i].content)) {
            if(st.empty() || st.top().empty()) {
                --global::in_expression;
                throw errors::MWSMessageException{"Invalid expression: " + str,global::get_line()};
            }
            auto op = operators[lexed[i].content];
            while(!ops.empty() && operators[ops.top()][0].priority >= op[0].priority) {
                GeneralTypeToken right;
                GeneralTypeToken left;
                if(st.top().size() == 1) {
                    right = st.top().front(); st.pop();
                }
                else {
                    lexed_tokens l(1);
                    for(auto i : st.top()) {
                        l[0].source.push_back(i.to_string());
                    }
                    ++global::in_compound;
                    right = run_lexed(l,false,false,-1,{},"",false,true);
                    --global::in_compound;
                    st.pop();
                }
                if(st.top().size() == 1) {
                    left = st.top().front(); st.pop();
                }
                else {
                    lexed_tokens l(1);
                    for(auto i : st.top()) {
                        l[0].source.push_back(i.to_string());
                    }
                    ++global::in_compound;
                    left = run_lexed(l,false,false,-1,{},"",false,true);
                    --global::in_compound;
                    st.pop();
                }

                Operator* roper;
                roper = get_operator(ops.top(),left.type,right.type);
                if(roper == nullptr) {
                    GeneralTypeToken l2 = tools::check4placeholder(left);
                    GeneralTypeToken r2 = tools::check4placeholder(right);
                    roper = get_operator(ops.top(),l2.type,right.type);
                    if(roper == nullptr) {
                        roper = get_operator(ops.top(),left.type,r2.type);
                        if(roper == nullptr) {
                            roper = get_operator(ops.top(),l2.type,r2.type);
                            right = r2;
                            left = l2;
                        }
                        else {
                            right = r2;
                        }
                    }
                    else {
                        left = l2;
                    }
                }
                if(roper == nullptr) {
                    --global::in_expression;
                    throw errors::MWSMessageException{"No overload of operator \"" + ops.top() + "\" matches the types: " + general_t2token(left.type).content + " | " + general_t2token(right.type).content,global::get_line()};
                }

                st.push({roper->parse(left,right)});
                ops.pop();
            }
            ops.push(lexed[i].content);
            f_op = true;
        }
        else {
            if(f_op) { st.push({}); f_op = false; }
            st.top().push_back(GeneralTypeToken(lexed[i]));
        }
    }


    while(!ops.empty()) {
        GeneralTypeToken left;
        GeneralTypeToken right;
        if(st.top().size() == 1) {
            right = st.top().front(); st.pop();
        }
        else {
            lexed_tokens l(1);
            for(auto i : st.top()) {
                l[0].source.push_back(i.to_string());
            }
            ++global::in_compound;
            right = run_lexed(l,false,false,-1,{},"",false,true);
            --global::in_compound;
            st.pop();
        }
        if(st.top().size() == 1) {
            left = st.top().front(); st.pop();
        }
        else {
            lexed_tokens l(1);
            for(auto i : st.top()) {
                l[0].source.push_back(i.to_string());
            }
            ++global::in_compound;
            left = run_lexed(l,false,false,-1,{},"",false,true);
            ++global::in_compound;
            st.pop();
        }

        Operator* roper;
        roper = get_operator(ops.top(),left.type,right.type);
        if(roper == nullptr) {
            GeneralTypeToken l2 = tools::check4placeholder(left);
            GeneralTypeToken r2 = tools::check4placeholder(right);
            roper = get_operator(ops.top(),l2.type,right.type);
            if(roper == nullptr) {
                roper = get_operator(ops.top(),left.type,r2.type);
                if(roper == nullptr) {
                    roper = get_operator(ops.top(),l2.type,r2.type);
                    right = r2;
                    left = l2;
                }
                else {
                    right = r2;
                }
            }
            else {
                left = l2;
            }
        }
        if(roper == nullptr) {
            --global::in_expression;
            throw errors::MWSMessageException{"No overload of operator \"" + ops.top() + "\" matches the types: " + general_t2token(left.type).content + " | " + general_t2token(right.type).content,global::get_line()};
        }

        st.push({roper->parse(left,right)});
        ops.pop();
    }
    --global::in_expression;
    if(st.size() != 1) {
        std::cout << "boo\n";
        throw errors::MWSMessageException{"Invalid expression: " + str,global::get_line()};
    }
    if(st.top().size() != 1) {
        lexed_tokens l(1);
        for(auto i : st.top()) {
            l[0].source.push_back(i.to_string());
        }
        ++global::in_compound;
        st.top() = {run_lexed(l,false,false,-1,{},"",false,true)};
        --global::in_compound;
    }
    if(st.top().front().type == General_type::VOID) {
        return Variable(Variable::Type::VOID);
    }
    return st.top().front().to_variable();
}

bool MeowScript::is_expression(std::string str) {
    if(!brace_check(str,'(',')')) {
        return false;
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
        return get_type(lexed[0],car_Command) != General_type::COMMAND;
    }

    bool f_op = false;
    bool f_any_ops = false;
    std::stack<std::string> ops;
    std::stack<std::vector<std::string>> st;
    st.push({});
    for(size_t i = 0; i < lexed.size(); ++i) {
        if(!lexed[i].in_quotes && is_operator(lexed[i].content)) {
            if(st.empty() || st.top().empty()) {
                return false;
            }
            while(!ops.empty()) {
                st.pop(); 
                ops.pop();
            }
            ops.push(lexed[i].content);
            f_op = true;
            f_any_ops = true;
        }
        else {
            if(f_op) { st.push({}); f_op = false; }
            st.top().push_back(lexed[i].content);
        }
    }

    return st.size() == ops.size()+1 && /*st.top().size() == 1 &&*/ f_any_ops;
}