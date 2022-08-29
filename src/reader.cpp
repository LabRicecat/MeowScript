#include "../inc/reader.hpp"
#include "../inc/expressions.hpp"
#include "../inc/global.hpp"

MEOWSCRIPT_SOURCE_FILE

std::streamsize MeowScript::get_flength(std::ifstream& file) {
    if(!file.is_open()) {
            return 0;
    }
    std::streampos temp_1 = file.tellg();
    file.seekg(0, std::fstream::end);
    std::streampos temp_2 = file.tellg();
    file.seekg(temp_1, std::fstream::beg);

    return temp_2;
}

std::string MeowScript::read(std::string path) {
    std::ifstream ifile;
    ifile.open(path, std::ios::binary);
    std::streamsize len = get_flength(ifile);
    char* dummy = new char[len];

    try {
        ifile.read(dummy, len);
    }
    catch(std::exception& err) {
        ifile.close();
        delete dummy;
        return "";
    }
    if (dummy == nullptr || strlen(dummy) == 0) {
        ifile.close();
        delete dummy;
        return "";
    }
    ifile.close();
    //dummy += '\0';
    std::string re;
    re.assign(dummy, len);

    delete[] dummy;
    dummy = nullptr;

    return re;
}

std::vector<Line> MeowScript::lex(std::string file) {
    return lex_text(read(file));
}

bool MeowScript::is_newline(char c) {
    switch(c) {
        case '\n':
#ifdef MEOWSCRIPT_WINDOWS
        case '\r\n':
#endif
        case ';':
            return true;
        default:
            return false;
    }
    return false;
}

bool MeowScript::is_open_brace(char c) {
    switch(c) {
        case '(':
        case '[':
        case '{':
            return true;
        default:
            return false;
    }
    return false;
}
bool MeowScript::is_closing_brace(char c) {
    switch(c) {
        case ')':
        case ']':
        case '}':
            return true;
        default:
            return false;
    }
    return false;
}

bool MeowScript::is_number(char c) {
    return c <= '9' && c >= '0';
}

bool MeowScript::is_capsuled_number(Token text) {
    if(text.in_quotes) {
        return false;
    }
    if(!brace_check(text,'(',')')) {
        return false;
    }
    text.content.erase(text.content.begin());
    text.content.erase(text.content.begin()+text.content.size()-1);

    auto lines = lex_text(text);
    std::vector<Token> lexed;
    for(auto i : lines)
        for(auto j : i.source)
            lexed.push_back(j);

    if(lexed.size() == 0) {
        return false;
    }
    bool found_op = false;
    bool b = false;
    for(auto i : lexed) {
        if(get_type(i) == General_type::NUMBER) {
            b = true;
        }
        else if((i.content == "-" || i.content == "+") && b) {
            return false;
        }
        else if(i.content == "-" || i.content == "+") {
            found_op = true;
        }
        else {
            return false;
        }
    }
    if(global::in_expression == 0 && global::in_argument_list == 0 && !found_op) {
        return false;
    }
    return true;
}

bool MeowScript::all_numbers(Token context, bool allow_dot) {
    if(context.in_quotes) {
        return false;
    }
    if(context.content.size() != 0 && context.content[0] == '-') {
        context.content.erase(context.content.begin());
    }
    if(context.content.size() == 0) {
        return false;
    }
    bool dot = false;
    for(auto i : context.content) {
        if(i == '.') {
            if(dot || !allow_dot) {
                return false;
            }
            dot = true;
        }
        else if(i > '9' || i < '0') {
            return false;
        }
    }
    return true;
}

bool MeowScript::is_valid_var_t(Token context) {
    std::vector<std::string> var_t_str = {
        "Number",
        "String",
        "List",
        "Any",
        "Void",
    };

    for(auto i : var_t_str) {
        if(context.content == i) {
            return true;
        }
    }
    return false;
}

bool MeowScript::is_valid_general_t(Token context) {
    std::vector<std::string> general_t_str = {
        "Number",
        "String",
        "List",
        "Function",
        "Command",
        "Compound",
        "Name",
        "Expression",
        "Argumentlist",
        "Operator",
        "Module",
        "Event",
        "Keyword",
        "Unknown",
        "Void"
    };
    for(auto i : general_t_str) {
        if(context.content == i) {
            return true;
        }
    }
    return false;
}

bool MeowScript::is_valid_operator_char(char ch) {
    return 
        ch == '+' ||
        ch == '-' ||
        ch == '~' ||
        ch == '=' ||
        ch == '<' ||
        ch == '>' ||
        ch == '^' ||
        ch == '!' ||
        ch == '?' ||
        ch == '*' ||
        ch == '/' ||
        ch == '%' ||
        ch == '.';
}

bool MeowScript::is_valid_operator_name(Token text) {
    for(auto i : text.content) {
        if(!is_valid_operator_char(i)) {
            return false;
        }
    }
    return true;
}

bool MeowScript::is_valid_name(Token context) {
    if(context.content == "" || context.in_quotes) {
        return false;
    }
    if(is_number(context.content.front())) {
        return false;
    }
    for(auto i : context.content) {
        if((i > 'z' || i < 'A' || (i > 'Z' && i < 'a')) && !is_number(i) && i != '_') {
            return false;
        }
    }
    return true;
}

bool MeowScript::is_known_keyword(Token text) {
    return (
        text.content == "public"  ||
        text.content == "private" ||
        text.content == "listen_only" ||
        text.content == "occur_only"
    );
}

bool MeowScript::is_valid_argumentlist(Token context) {
    if(!brace_check(context,'(',')')) {
        return false;
    }
    context.content.erase(context.content.begin());
    context.content.erase(context.content.begin()+context.content.size()-1);

    if(context.content.size() == 0) {
        return true;
    }
    bool tm = true;
    int in_br = false;
    bool in_q = false;
    for(auto i : context.content) {
        if(i == ',' && tm && in_br == 0 && !in_q) {
            return false;
        }
        else if(i == ',' && in_br == 0 && !in_q) {
            tm = true;
        }
        else if(i == '"' && in_br == 0) {
            in_q = !in_q;
            tm = false;
        }
        else if(is_open_brace(i) && !in_q) {
            ++in_br;
            tm = false;
        }
        else if(is_closing_brace(i) && !in_q) {
            --in_br;
            tm = false;
        }
        else if(is_valid_operator_char(i) && !in_q && in_br == 0) {
            return false;
        }
        else if(i != ' ' && i != '\t' && !is_newline(i)) {
            tm = false;
        }
    }
    if(in_q || in_br || tm) {
        return false;
    }
    return true;
}

bool MeowScript::is_brace_pair(char open,char close) {
    return (open=='('&&close==')') || (open=='['&&close==']') || (open=='{'&&close=='}');
}

bool MeowScript::brace_check(Token context, char open, char close) {
    if(context.in_quotes) {
        return false;
    }

    int brace_counter = 0;
    bool in_quote = false;
    bool until_eoc = false;
    bool until_nl = false;
    for(size_t i = 0; i < context.content.size(); ++i) {
        if(context.content[i] == '"') {
            in_quote = !in_quote;
        }
        else if(context.content[i] == open && !in_quote) {
            ++brace_counter;
        }
        else if(context.content[i] == close && !in_quote && !until_eoc && !until_nl) {
            --brace_counter;
        }
        else if(context.content[i] == '#' && !in_quote) {
            if(context.content.size() != i+1 && context.content[i+1] == '#') {
                until_eoc = !until_eoc;
                ++i;
            }
            else if(!until_eoc) {
                until_nl = true;
            }
        }
        else if(is_newline(context.content[i])) {
            until_nl = false;
        }
        
        if(brace_counter == 0 && i+1 >= context.content.size() && context.content[i] != close) {
            return false;
        }
    }
    return brace_counter == 0;
}

std::vector<Line> MeowScript::lex_text(std::string source) {
    std::vector<Line> ret;

    bool in_quote = false;
    std::stack<char> in_braces;
    bool until_nl = false;
    bool until_eoc = false;
    uint32_t line_counter = 1;
    Line tmp_line;
    tmp_line.line_count = 1;
    Token tmp_token;
    for(size_t i = 0; i < source.size(); ++i) {
        if(is_newline(source[i]) && !in_quote && in_braces.empty() && !until_eoc) {
            if(tmp_token.content != "" || tmp_token.in_quotes) {
                tmp_line.source.push_back(tmp_token);
            }
            ret.push_back(tmp_line);

            tmp_line.source.clear();
            tmp_token.content = "";
            tmp_token.in_quotes = false;
            if(source[i] != ';') {
                ++line_counter;
            }
            tmp_line.line_count = line_counter;
            until_nl = false;
        }
        else if(is_newline(source[i]) && !until_eoc) {
            tmp_token.content += source[i];
            ++line_counter;
            until_nl = false;
        }
        else if(is_open_brace(source[i]) && !in_quote && !until_eoc && !until_nl) {
            // So that things like add(1,1) is lexed as {"add","(1,1)"} !
            if((tmp_token.content != "" || tmp_token.in_quotes) && in_braces.empty()) {
                tmp_line.source.push_back(tmp_token);
                tmp_token.content = "";
                tmp_token.in_quotes = false;
            }
            tmp_token.content += source[i];
            in_braces.push(source[i]);
        }
        else if(is_closing_brace(source[i]) && !in_quote && !until_eoc && !until_nl) {
            if(in_braces.empty() || !is_brace_pair(in_braces.top(),source[i])) {
                std::string err = "Unexpected token: " + std::string(1,source[i]);
                throw errors::MWSMessageException{err,line_counter};
            }
            tmp_token.content += source[i];
            in_braces.pop();
        }
        else if(source[i] == '\\' && in_braces.empty()) {
            if(i != source.size()-1) {
                if(source[i+1] == '"') {
                    tmp_token.content += '"';
                    ++i;
                }
                else if(source[i+1] == '\\') {
                    tmp_token.content += '\\';
                    ++i;
                }
                else if(source[i+1] == 't') {
                    tmp_token.content += "\t";
                    ++i;
                }
                else if(source[i+1] == 'r') {
                    tmp_token.content += "\r";
                    ++i;
                }
                else if(source[i+1] == 'n' && in_quote) {
                    tmp_token.content += '\n';
                    ++i;
                }
                else {
                    tmp_token.content += '\\';
                }
            }
            //else {
            //    tmp_token.content += '\\';
            //}
        }
        else if(source[i] == '"' && !until_eoc && !until_nl && in_braces.empty()) {
            if(in_quote) {
                in_quote = false;
                if(source.size()-1 != i && source[i+1] != '\t' && source[i+1] != ' ' && source[i+1] != ',' && source[i+1] != ';' && !is_newline(source[i+1]) && !is_valid_operator_char(source[i+1])) {
                    throw errors::MWSMessageException{"Unexpected token: " + std::string(1,source[i]),line_counter};
                }
            }
            else {
                tmp_token.in_quotes = true;
                in_quote = true;
            }
        }
        else if((source[i] == ' ' || source[i] == '\t' || source[i] == ',') && !in_quote && in_braces.empty() && !until_eoc && !until_nl) {
            if(tmp_token.content != "" || tmp_token.in_quotes) {
                tmp_line.source.push_back(tmp_token);
                tmp_token.content = "";
                tmp_token.in_quotes = false;
                if(source[i] == ',') {
                    tmp_line.source.push_back(",");
                }
            }
        }
        else if(is_valid_operator_char(source[i]) && !in_quote && in_braces.empty() && !until_eoc && !until_nl) {
            if(tmp_token.content != "" || tmp_token.in_quotes) {
                if(tmp_token.in_quotes && source[i-1] != '"') {
                    throw errors::MWSMessageException{"Unexpected token: " + std::string(1,source[i]),line_counter};
                }
                tmp_line.source.push_back(tmp_token);
                tmp_token.content = "";
                tmp_token.in_quotes = false;
            }
            tmp_line.source.push_back(std::string(1,source[i]));
        }
        else if(source[i] == '#' && !in_quote) {
            if(source.size() != i+1 && source[i+1] == '#') {
                if(until_eoc) {
                    if(tmp_token.content != "" || tmp_token.in_quotes) {
                        tmp_line.source.push_back(tmp_token);
                    }
                    ret.push_back(tmp_line);

                    tmp_line.source.clear();
                    tmp_token.content = "";
                    tmp_token.in_quotes = false;
                    if(source[i] != ';') {
                        ++line_counter;
                    }
                    tmp_line.line_count = line_counter;
                }
                until_eoc = !until_eoc;
                ++i;
            }
            else if(!until_eoc) {
                until_nl = true;
            }
        }
        else if(!until_eoc && !until_nl) {
            tmp_token.content += source[i];
        }
    }
    if(tmp_token.content != "" || tmp_token.in_quotes) {
        tmp_line.source.push_back(tmp_token);
    }
    if(!tmp_line.source.empty()) {
        ret.push_back(tmp_line);
    }
    std::vector<Line> tm_ret;
    for(auto line : ret) {
        Line ln;
        ln.line_count = line.line_count;
        for(size_t i = 0; i < line.source.size(); ++i) {
            if(line.source[i].in_quotes || line.source[i].content.size() != 1 || !is_valid_operator_char(line.source[i].content[0]) || ln.source.size() == 0) {
                ln.source.push_back(line.source[i]);
            }
            else {
                // Is an operator and could be merged!
                if(ln.source.back().content.size() > 0 && !ln.source.back().in_quotes && is_valid_operator_char(ln.source.back().content[0])) {
                    ln.source.back().content.push_back(line.source[i].content[0]);
                }
                else {
                    ln.source.push_back(line.source[i]);
                }
            }
        }
        tm_ret.push_back(ln);
    }
    for(auto& i : tm_ret) {
        for(size_t j = 0; j < i.source.size(); ++j) {
            if(j != 0 && j != i.source.size()-1
                 && i.source[j].content == "." 
                 && all_numbers(i.source[j-1],false)
                 && all_numbers(i.source[j+1],false)
            ) {
                i.source[j-1].content = i.source[j-1].content + "." + i.source[j+1].content;
                i.source.erase(i.source.begin()+j);
                i.source.erase(i.source.begin()+j);
            }
        }
    }


    return tm_ret;
}