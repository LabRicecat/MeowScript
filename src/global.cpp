#include "../inc/global.hpp"

MEOWSCRIPT_SOURCE_FILE

unsigned int global::get_line() {
    if(global::line_count.empty()) {
        return 0;
    }
    return global::line_count.top();
}