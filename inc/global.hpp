#ifndef MEOWSCRIPT_IG_GLOBAL_HPP
#define MEOWSCRIPT_IG_GLOBAL_HPP

#include "defs.hpp"

#include <map>
#include <filesystem>
#include <stack>

namespace fs = std::filesystem;

MEOWSCRIPT_HEADER_BEGIN

namespace global {
    inline std::stack<fs::path> include_path;
    inline int runner_should_return = 0;
    inline int runner_should_exit = 0;
    inline int in_compound = 0;
    inline int in_expression = 0;
    inline int in_argument_list = 0;
    inline int in_loop = 0;
    inline int break_loop = 0;
    inline int continue_loop = 0;
    inline std::stack<unsigned int> line_count;
    unsigned int get_line();
}

MEOWSCRIPT_HEADER_END

#endif