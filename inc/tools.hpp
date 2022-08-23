#ifndef MEOWSCRIPT_IG_TOOLS_HPP
#define MEOWSCRIPT_IG_TOOLS_HPP

#include "defs.hpp"
#include "reader.hpp"

#include <tuple>

MEOWSCRIPT_HEADER_BEGIN

struct GeneralTypeToken;

using argument_list = std::vector<GeneralTypeToken>;
namespace tools {
    argument_list parse_argument_list(GeneralTypeToken context);
    argument_list parse_argument_list(Token context);

    // Checks if token could be a variable name and if yes \
    // tries to convert this variable into a GeneralTypeToken and returns it.
    // If it fails, returns `token`
    GeneralTypeToken check4var(GeneralTypeToken token);

    // Checks if token is a runnable compound and tries to run it and return it's result.
    GeneralTypeToken check4compound(GeneralTypeToken token);

    // Runs check4compound and chack4var in that order
    GeneralTypeToken check4placeholder(GeneralTypeToken token);

    GeneralTypeToken check4expression(GeneralTypeToken token);

    Token remove_unneeded_chars(Token token);

    Token remove_uness_decs(Token num, bool to_int);
}

MEOWSCRIPT_HEADER_END

#endif