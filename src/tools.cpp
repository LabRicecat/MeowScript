#include "../inc/tools.hpp"
#include "../inc/errors.hpp"
#include "../inc/runner.hpp"
#include "../inc/expressions.hpp"

MEOWSCRIPT_SOURCE_FILE

argument_list MeowScript::tools::parse_argument_list(GeneralTypeToken context) {
    return parse_argument_list(context.source);
}

argument_list MeowScript::tools::parse_argument_list(Token context) {
    argument_list ret;
    if(context.content == "" || context.in_quotes || !brace_check(context,'(',')')) {
        return argument_list();
    }
    context.content.erase(context.content.begin());
    context.content.erase(context.content.begin()+context.content.size()-1);
    if(context.content.size() == 0) {
        return argument_list();
    }

    Token tk;
    int in_br = false;
    bool in_q = false;
    ++global::in_argument_list;
    for(auto i : context.content) {
        if(i == ',' && in_br == 0 && !in_q) {
            if(tk.content == "" && !tk.in_quotes) {
                --global::in_argument_list;
                return argument_list();
            }
            ret.push_back(GeneralTypeToken(tk));

            tk.content = "";
            tk.in_quotes = false;
        }
        else if(i == '"' && in_br == 0) {
            in_q = !in_q;
            tk.content += i;
        }
        else if(is_open_brace(i) && !in_q) {
            ++in_br;
            tk.content += i;
        }
        else if(is_closing_brace(i) && !in_q) {
            --in_br;
            tk.content += i;
        }
        else if(!is_newline(i) && i != ' ' && i != '\t' || in_br != 0 || in_q) {
            tk.content += i;
        }
    }
    if(tk.content != "" || tk.in_quotes) {
        ret.push_back(GeneralTypeToken(tk));
    }
    --global::in_argument_list;
    if(in_q || in_br) {
        --global::in_argument_list;
    }

    return ret;
}

GeneralTypeToken MeowScript::tools::check4var(GeneralTypeToken token) {
    if(token.type == General_type::NAME) {
        Variable* var = get_variable(token.source);
        if(var != nullptr) {
            return GeneralTypeToken(*var);
        }
    }
    return token;
}

GeneralTypeToken MeowScript::tools::check4compound(GeneralTypeToken token) {
    if(token.type == General_type::COMPOUND) {
        token.source.content.erase(token.source.content.begin());
        //token.source = remove_unneeded_chars(token.source);
        token.source.content.erase(token.source.content.begin()+token.source.content.size()-1);
        //token.source = remove_unneeded_chars(token.source);
        ++global::in_compound;
        GeneralTypeToken ret = run_text(token.source,false);
        --global::in_compound;
        //if(ret.type == General_type::VOID) {
        //    throw errors::MWSMessageException{"Compound did not return anything!",global::get_line()};
        //}
        return ret;
    }
    return token;
}

GeneralTypeToken MeowScript::tools::check4placeholder(GeneralTypeToken token) {
    return check4var(check4compound(check4expression(token)));
}

GeneralTypeToken MeowScript::tools::check4expression(GeneralTypeToken token) {
    if(token.type == General_type::EXPRESSION) {
        Variable ret = parse_expression(token.to_string());
        return GeneralTypeToken(ret);
    }
    return token;
}

Token MeowScript::tools::remove_unneeded_chars(Token token) {
    if(token.in_quotes) {
        return token;
    }
    size_t i = 0;
    for(i = 0; i < token.content.size(); ++i) {
        if(token.content[i] != ' ' && token.content[i] != '\t' && !is_newline(token.content[i])) {
            break;
        }
    }
    token.content.erase(token.content.begin(),token.content.begin()+i);
    for(i = token.content.size(); i != -1; --i) {
        if(token.content[i] != ' ' && token.content[i] != '\t' && !is_newline(token.content[i])) {
            break;
        }
    }
    token.content.erase(token.content.begin()+i,token.content.end());
    return token;
}


Token MeowScript::tools::remove_uness_decs(Token num, bool to_int) {
    if(num.in_quotes) {
        return num;
    }
    bool has_dot = false;
    bool all_zero = true;
    for(auto i : num.content) {
        //                      vvvvvvvv - just in case
        all_zero &= (i == '0' || i == '.');
        if(i == '.') has_dot = true;
    }
    if(all_zero) {
        return "0";
    }
    if(!has_dot) {
        return num;
    }

    if(to_int) {
        for(size_t i = 0; i < num.content.size(); ++i) {
            if(num.content[i] == '.') {
                num.content.erase(num.content.begin()+i,num.content.end());
                return num;
            }
        }
        return num;
    }

    for(size_t i = num.content.size()-1; i != -1; --i) {
        if(num.content[i] == '.') {
            num.content.erase(num.content.begin()+i);
            break;
        }
        else if(num.content[i] == '0') {
            num.content.erase(num.content.begin()+i);
        }
        else {
            break;
        }
    }

    return num;
}
