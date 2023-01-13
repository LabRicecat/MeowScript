#include "../inc/reader.hpp"
#include "../inc/expressions.hpp"
#include "../inc/global.hpp"
#include "../inc/kittenlexer.hpp"

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
        "Dictionary",
        "Object",
        "Function",
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
        "Parameterlist,"
        "Operator",
        "Module",
        "Event",
        "Keyword",
        "Dictionary",
        "Struct",
        "Object",
        "Typename",
        "Unknown",
        "Void",
        "FUNCCALL",
    };
    for(auto i : general_t_str) {
        if(context.content == i) {
            return true;
        }
    }
    return false;
}

bool MeowScript::is_operator_begin(std::string s) {
    for(auto i : operator_names) {
        if(i.size() < s.size()) continue;
        bool b = false;
        for(size_t j = 0; j < s.size(); ++j) {
            if(i[j] != s[j]) { b = true; break; }
        }
        if(!b) return true;
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
        ch == ':' ||
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
    if(context.content == "" || context.in_quotes || !brace_check(context,'(',')')) {
        return false;
    }
    context.content.erase(context.content.begin());
    context.content.erase(context.content.begin()+context.content.size()-1);

    auto lexed = lex_text(context.content);
    if(lexed.empty() || (lexed.size() == 1 && lexed[0].source.empty())) {
        return true;
    }

    std::vector<Token> line;
    for(auto i : lexed)
        for(auto j : i.source) 
            line.push_back(j);
    
    int last = 0;

    for(auto i : line) {
        if(i.content == "," && !i.in_quotes) {
            if(last == 0) {
                return false;
            }
            last = 0;
        }
        else {
            ++last;
        }
    }

    return true;
}

bool MeowScript::is_valid_parameterlist(Token context) {
    if(!brace_check(context,'(',')')) {
        return false;
    }

    context.content.erase(context.content.begin());
    context.content.erase(context.content.begin()+context.content.size()-1);
    
    auto lines = lex_text(context.content);
    if(lines.empty() || (lines.size() == 1 && lines[0].source.empty())) {
        return true;
    }
    std::vector<Parameter> ret;
    std::vector<Token> line;
    for(auto i : lines) {
        for(auto j : i.source) {
            line.push_back(j);
        }
    }

    std::vector<std::vector<Token>> arguments;
    if(line.size() != 0)
        arguments.push_back({});
    for(size_t i = 0; i < line.size(); ++i) {
        if(line[i].content == ",") {
            if(arguments.back().empty()) {
                return false;
            }
            arguments.push_back({});
        }
        else {
            arguments.back().push_back(line[i]);
        }
    }

    for(auto i : arguments) {
        for(size_t j = 0; j < i.size(); ++j) {
            if(i[j].content == "" && !i[j].in_quotes) {
                i.erase(i.begin()+j);
                --j;
            }
        }

        if(i.size() == 1) {
            if(!is_valid_name(i[0]) && !is_literal_value(i[0]) /*|| is_variable(context) || is_struct(context) || is_function(context)*/) {
                return false;
            }
        }
        else if(i.size() == 2) {
            if(!is_valid_name(i[0]) && !is_literal_value(i[0])) {
                return false;
            }
            if(!is_ruleset(i[1])) {
                return false;
            }
        }
        else if(i.size() == 3) {
            if(!is_valid_name(i[0]) && !is_literal_value(i[0]) /*|| is_variable(context) || is_struct(context) || is_function(context)*/) {
                return false;
            }
            if(i[1].content != "::" && i[1].content != ":") {
                return false;
            }
        }
        else if(i.size() == 4) {
            if(!is_valid_name(i[0]) && !is_literal_value(i[0])) {
                return false;
            }
            if(!is_ruleset(i[1])) {
                return false;
            }
            if(i[2].content != "::" && i[2].content != ":") {
                return false;
            }
        }
        else {
            return false;
        }
    }
    return true;
}

bool MeowScript::is_brace_pair(char open,char close) {
    return (open=='('&&close==')') || (open=='['&&close==']') || (open=='{'&&close=='}');
}

bool MeowScript::is_dictionary(Token context) {
    if(context.in_quotes || context.content == "") {
        return false;
    }
    if(!brace_check(context,'{','}')) {
        return false;
    }
    context.content.erase(context.content.begin());
    context.content.erase(context.content.end()-1);
    
    auto lines = lex_text(context.content);
    std::vector<Token> lexed;
    for(auto i : lines)
        for(auto j : i.source)
            lexed.push_back(j);
    
    if(lexed.size() == 0) {
        return true;
    }

    GeneralTypeToken key;
    GeneralTypeToken value;
    bool got_equals = false;
    
    for(auto i : lexed) {
        if(!i.in_quotes && i.content == "=") {
            if(got_equals || key == general_null) {
                return false;
            }
            got_equals = true;
        }
        else if(key == general_null) {
            key = GeneralTypeToken(i);
        }
        else if(got_equals && value == general_null) {
            value = GeneralTypeToken(i);
        }
        else if(!i.in_quotes && (i.content == "," || i.content == ";")) {
            if(!got_equals || key == general_null || key.type == General_type::UNKNOWN || value == general_null || value.type == General_type::UNKNOWN) {
                return false;
            }
            got_equals = false;
            key = general_null;
            value = general_null;
        }
        else if(tools::remove_unneeded_chars(i.content).content != "") {
            return false;
        }
    }
    return !(!got_equals || key == general_null || key.type == General_type::UNKNOWN || value == general_null || value.type == General_type::UNKNOWN);
}

bool MeowScript::is_literal_value(Token context) {
    return (
        car_Number |
        car_String |
        car_List |
        car_Dictionary
    ).matches(get_type(context));
}

bool MeowScript::is_valid_function_return(Token context) {
    return is_valid_var_t(context) || is_struct(context) || is_funcparam_literal(context);
}

bool MeowScript::in_any_braces(Token context) {
    return brace_check(context,'(',')') || brace_check(context,'[',']') || brace_check(context,'{','}');
}

bool MeowScript::is_function_call(Token context) {
    if(context.in_quotes) return false;
    KittenLexer lexer = KittenLexer()
        .add_capsule('(',')')
        .add_capsule('{','}')
        .add_capsule('[',']')
        .add_con_extract(is_valid_operator_char)
        .add_ignore(' ')
        .add_ignore('\n')
        .erase_empty()
        ;
    auto lexed = lexer.lex(context.content);
    if(lexed.size() != 3 && lexed.size() != 2) return false;
    if(!is_valid_name(lexed[0].src) && !is_function_literal(lexed[0].src)) return false;
    if(!is_valid_argumentlist(lexed[1].src)) return false;
    return (lexed.size() == 3 && lexed[2].src == "!" && !lexed[2].str) || lexed.size() == 2;
}

bool MeowScript::brace_check(Token context, char open, char close) {
    if(context.in_quotes) {
        return false;
    }

    int brace_counter = 0;
    bool in_brace_until_quote = false;
    bool in_quote = false;
    bool until_eoc = false;
    bool until_nl = false;
    for(size_t i = 0; i < context.content.size(); ++i) {
        if(context.content[i] == '"' && !until_eoc && !until_nl) {
            if(brace_counter == 1) {
                in_quote = !in_quote;
            }
            else {
                in_brace_until_quote = !in_brace_until_quote;
            }
        }
        else if(context.content[i] == open && !in_quote && !until_eoc && !until_nl && !in_brace_until_quote) {
            ++brace_counter;
        }
        else if(context.content[i] == close && !in_quote && !until_eoc && !until_nl && !in_brace_until_quote) {
            --brace_counter;
        }
        else if(context.content[i] == '#' && !in_quote && !in_brace_until_quote) {
            if(context.content.size() != i+1 && context.content[i+1] == '#') {
                until_eoc = !until_eoc;
                ++i;
                until_nl = false;
            }
            else if(!until_eoc) {
                until_nl = true;
            }
        }
        else if(is_newline(context.content[i]) && (until_nl && context.content[i] != ';') && !in_quote && !in_brace_until_quote) {
            until_nl = false;
        }
        
        if(brace_counter == 0 && !(i+1 >= context.content.size() && context.content[i] == close)) {
            return false;
        }
    }
    return brace_counter == 0;
}

std::vector<Line> MeowScript::lex_text_old(std::string source) {
    std::vector<Line> ret;
    bool in_quote = false;
    std::stack<char> in_braces;
    bool until_nl = false;
    bool until_eoc = false;
    bool in_brace_until_quote = false;
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
        else if(is_newline(source[i]) && (until_nl && source[i] != ';') && !until_eoc) {
            tmp_token.content += source[i];
            ++line_counter;
            until_nl = false;
        }
        else if(is_open_brace(source[i]) && !in_quote && !until_eoc && !until_nl && !in_brace_until_quote) {
            // So that things like add(1,1) is lexed as {"add","(1,1)"} !
            if((tmp_token.content != "" || tmp_token.in_quotes) && in_braces.empty()) {
                tmp_line.source.push_back(tmp_token);
                tmp_token.content = "";
                tmp_token.in_quotes = false;
            }
            tmp_token.content += source[i];
            in_braces.push(source[i]);
        }
        else if(is_closing_brace(source[i]) && !in_quote && !until_eoc && !until_nl && !in_brace_until_quote) {
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
        else if(source[i] == '"' && !until_eoc && !until_nl) {
            if(in_braces.empty()) {
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
            else {
                in_brace_until_quote = !in_brace_until_quote;
                tmp_token.content += source[i];
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
        else if(source[i] == '#' && !in_quote && !in_brace_until_quote) {
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
        std::string merges;
        for(size_t i = 0; i < line.source.size(); ++i) {
            if(line.source[i].in_quotes || line.source[i].content.size() != 1 || !is_valid_operator_char(line.source[i].content[0]) || ln.source.size() == 0) {
                if(merges != "") {
                    ln.source.push_back(merges);
                    merges = "";
                }
                ln.source.push_back(line.source[i]);
            }
            else {
                // Is an operator and could be merged!
                if(is_operator_begin(merges + std::string(1,line.source[i].content[0]))) {
                    merges.push_back(line.source[i].content[0]);
                }
                else {
                    if(merges != "") {
                        ln.source.push_back(merges);
                        merges = "";
                    }
                    ln.source.push_back(line.source[i]);
                }
            }
        }
        tm_ret.push_back(ln);
    }
    for(auto& i : tm_ret) {
        for(auto& j : i.source)
            j = tools::check4replace(j);
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

std::vector<Line> MeowScript::lex_text(std::string source) {
    std::vector<Line> ret;
    KittenLexer lexer;
    lexer.
        add_capsule('(',')')
        .add_capsule('{','}')
        .add_capsule('[',']')
        .add_stringq('\'')
        .add_stringq('"')
        .add_linebreak('\n')
        .add_linebreak(';')
        .add_ignore(' ')
        .add_ignore('\t')
        .add_lineskip('#')
        .add_con_extract(is_valid_operator_char)
        .add_extract(',')
        .add_backslashopt('"','\"')
        .add_backslashopt('\\','\\')
        .add_backslashopt('n','\n')
        .add_backslashopt('t','\t')
        .add_backslashopt('b','\b')
        .add_backslashopt('r','\r')
        .erase_empty()
    ;
    auto lexed = lexer.lex(source);
    if(!lexer) {
        if(lexed.size() != 0)
            throw errors::MWSMessageException{"Brace missmatch at " + std::to_string(lexed.back().line),0};
        else 
            throw errors::MWSMessageException{"Brace missmatch!",0};
    }
    ret.push_back(Line{{},1});
    for(auto i : lexed) {
        if(ret.back().line_count != i.line) {
            ret.push_back(Line{{},(unsigned int)i.line});
        }
        Token tk;
        tk.in_quotes = i.str;
        tk.content = i.src;
        tk.line = i.line;
        ret.back().source.push_back(tk);
    }
    std::vector<Line> tm_ret;
    for(auto line : ret) {
        Line ln;
        ln.line_count = line.line_count;
        std::string merges;
        for(size_t i = 0; i < line.source.size(); ++i) {
            if(line.source[i].in_quotes || line.source[i].content.size() != 1 || !is_valid_operator_char(line.source[i].content[0]) || ln.source.size() == 0) {
                if(merges != "") {
                    ln.source.push_back(merges);
                    merges = "";
                }
                ln.source.push_back(line.source[i]);
            }
            else {
                // Is an operator and could be merged!
                if(is_operator_begin(merges + std::string(1,line.source[i].content[0]))) {
                    merges.push_back(line.source[i].content[0]);
                }
                else {
                    if(merges != "") {
                        ln.source.push_back(merges);
                        merges = "";
                    }
                    ln.source.push_back(line.source[i]);
                }
            }
        }
        tm_ret.push_back(ln);
    }
    for(auto& i : tm_ret) {
        for(auto& j : i.source)
            j = tools::check4replace(j);
    }
    // TODO: add "." as operator
    return tm_ret;
}