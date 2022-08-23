#ifndef MEOWSCRIPT_IG_FUNCTIONS_HPP
#define MEOWSCRIPT_IG_FUNCTIONS_HPP

#include "defs.hpp"
#include "variables.hpp"
#include "errors.hpp"

#include <map>

MEOWSCRIPT_HEADER_BEGIN

class Function {
public:
    std::vector<Line> body;
    std::vector<Variable::Type> args;
    std::vector<std::string> arg_names;
    unsigned int scope_idx = 0;
    Variable::Type return_type = Variable::Type::UNKNOWN;
    std::filesystem::path file;
    // Takes care of the required amount of arguments and their types as well as the return value
    // Throws `MWSMessageException` on error
    Variable run(std::vector<Variable> args);
};

struct Event {
    std::vector<Function> listeners;
    std::string from_file;
    enum class Visibility {
        PUBLIC,
        PRIVATE,
        LISTEN_ONLY,
        OCCUR_ONLY
    }visibility = Visibility::PUBLIC;

    std::vector<std::string> arg_names;
    std::vector<Variable::Type> arg_types;
};

namespace global {
    inline std::map<std::string,Event> events;
}

MEOWSCRIPT_HEADER_END

#endif