#ifndef MEOWSCRIPT_IG_READER_HPP
#define MEOWSCRIPT_IG_READER_HPP

#include "defs.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <vector>

MEOWSCRIPT_HEADER_BEGIN

std::streamsize get_flength(std::ifstream& file);

std::string read(std::string path);

struct Token {
    bool in_quotes = false;
    std::string content;

    operator std::string() {
        return content;
    }

    Token(std::string str) {
        content = str;
    }
    Token(const char* cstr) {
        content = std::string(cstr);
    }
    Token() {}

    void operator=(std::string str) {
        in_quotes = false;
        content = str;
    }
};

struct Line {
    std::vector<Token> source;
    uint32_t line_count = 0;
    operator std::vector<Token>() {
        return source;
    }
    
};

bool is_newline(char c);
bool is_open_brace(char c);
bool is_closing_brace(char c);
bool is_number(char c);
bool is_capsuled_number(Token text);
bool all_numbers(Token text, bool allow_dot = true);
bool is_valid_general_t(Token text);
bool is_valid_var_t(Token text);
bool is_valid_operator_char(char ch);
bool is_valid_operator_name(Token text);
bool is_valid_name(Token text);
bool is_valid_argumentlist(Token context);
bool is_brace_pair(char open,char close);

using lexed_tokens = std::vector<Line>;

lexed_tokens lex(std::string file);
lexed_tokens lex_text(std::string source);

bool brace_check(Token context, char open, char close);

enum class General_type {
    NUMBER,
    STRING,
    LIST,
    FUNCTION,
    COMMAND,
    COMPOUND,
    NAME,
    EXPRESSION,
    ARGUMENTLIST,
    OPERATOR,
    MODULE,
    UNKNOWN,
    VOID
};

MEOWSCRIPT_HEADER_END

#endif