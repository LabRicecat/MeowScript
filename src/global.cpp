#include "../inc/global.hpp"

MEOWSCRIPT_SOURCE_FILE

unsigned int MeowScript::global::get_line() {
    if(global::line_count.empty()) {
        return 0;
    }
    return global::line_count.top();
}

void MeowScript::global::add_trace(unsigned int line, std::string name,std::string file) {
    call_trace.push(std::make_tuple(line,name,file));
}

bool MeowScript::global::pop_trace() {
    if(!call_trace.empty()) {
        call_trace.pop();
        return true;
    }
    return false;
}