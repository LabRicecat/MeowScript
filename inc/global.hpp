#ifndef MEOWSCRIPT_IG_GLOBAL_HPP
#define MEOWSCRIPT_IG_GLOBAL_HPP

#include "defs.hpp"

#include <map>
#include <filesystem>
#include <stack>
#include <vector>

namespace fs = std::filesystem;

MEOWSCRIPT_HEADER_BEGIN

struct Variable;

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
    inline std::vector<std::string> imported_files;
    bool is_imported(fs::path file);
    unsigned int get_line();

    inline std::vector<Variable> args;

    inline std::stack<std::tuple<unsigned int,std::string,std::string>> call_trace;
    void add_trace(unsigned int line, std::string name,std::string file);
    bool pop_trace();

    inline std::string origin_file = "";

    fs::path include_parent_path();

    inline int in_struct = 0;
}


MEOWSCRIPT_HEADER_END

#endif