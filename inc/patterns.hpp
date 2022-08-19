#ifndef MEOWSCRIPT_IG_PATTERNS_HPP
#define MEOWSCRIPT_IG_PATTERNS_HPP

#include "defs.hpp"
#include "variables.hpp"
#include "commands.hpp"

#include <map>

MEOWSCRIPT_HEADER_BEGIN

class Pattern {
    std::map<char,CommandArgReqirement> checks;
    std::vector<char> forced;
    void (*handler)(char identifyer, int num);
    std::string pattern;

public:
    Pattern() {}
    Pattern(std::string token);

    Pattern& match_command(char ident, CommandArgReqirement reqirement, bool force = false);
    Pattern& set_replace(void (*handler)(char identifyer, int num));
    Pattern& set_pattern(std::string pattern);

    bool does_match(std::string literal);
    Token replace(Token);
};

MEOWSCRIPT_HEADER_END

#endif