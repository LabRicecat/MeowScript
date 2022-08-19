#include "../inc/patterns.hpp"

MEOWSCRIPT_SOURCE_FILE

Pattern& MeowScript::Pattern::match_command(char ident, CommandArgReqirement reqirement, bool force) {
    checks[ident] = reqirement;
    if(force) {
        forced.push_back(ident);
    }
    return *this;
}

Pattern& MeowScript::Pattern::set_replace(void (*handler)(char identifyer, int num)) {
    this->handler = handler;
    return *this;
}

Pattern& MeowScript::Pattern::set_pattern(std::string pattern) {
    this->pattern = pattern;
    return *this;
}

bool MeowScript::Pattern::does_match(std::string literal) {
    return false;
}

Token MeowScript::Pattern::replace(Token tk) {
    Token ret;
    for(auto i : tk.content) {
        if(i == '%') {

        }
        else {
            
        }
    }
    return Token();
}