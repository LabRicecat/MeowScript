#ifndef MEOWSCRIPT_IG_FUNCTIONS_HPP
#define MEOWSCRIPT_IG_FUNCTIONS_HPP

#include "defs.hpp"
#include "variables.hpp"
#include "errors.hpp"

#include <map>
#include <tuple>
#include <string>

MEOWSCRIPT_HEADER_BEGIN

struct Parameter {
    Variable::Type type = Variable::Type::ANY;
    std::string name;
    std::string struct_name;

    Parameter() {}
    Parameter(Variable::Type type) : type(type) {}
    Parameter(Variable::Type type, std::string name) : type(type), name(name) {}
    Parameter(Variable::Type type, std::string name, std::string struct_name) : type(type), name(name), struct_name(struct_name) {}
    void operator=(Variable::Type type) {
        this->type = type;
    }

    bool matches(Variable var) const;
    bool matches(Parameter param) const;

    void operator=(Parameter p) {
        this->type = p.type;
        this->name = p.name;
        this->struct_name = p.struct_name;
    }
};

bool paramlist_matches(std::vector<Parameter> params1,std::vector<Parameter> params2);

class Function {
public:
    using ReturnType = Parameter;
    std::vector<Line> body;
    std::vector<Parameter> params;
    unsigned int scope_idx = 0;
    ReturnType return_type = Variable::Type::UNKNOWN;
    fs::path file;
    // Takes care of the required amount of arguments and their types as well as the return value
    // Throws `MWSMessageException` on error
    Variable run(std::vector<Variable> args, bool method_mode = false);
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

    std::vector<Parameter> params;
};

namespace global {
    inline std::map<std::string,Event> events;
}

MEOWSCRIPT_HEADER_END

#endif