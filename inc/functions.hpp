#ifndef MEOWSCRIPT_IG_FUNCTIONS_HPP
#define MEOWSCRIPT_IG_FUNCTIONS_HPP

#include "defs.hpp"
#include "variables.hpp"
#include "errors.hpp"

#include <map>
#include <tuple>
#include <string>
#include <memory>

MEOWSCRIPT_HEADER_BEGIN

struct Operator;
struct ArgRule {
    Operator* operat;
    Variable value;
    std::string op_name = "";

    bool applies(Variable var) const;

    bool operator==(ArgRule r);
};

using RuleSet = std::vector<ArgRule>;
struct Function;
struct Parameter {
private:
    std::unique_ptr<Function> m_fntmpl = nullptr;
public:
    Variable::Type type = Variable::Type::ANY;
    std::string name;
    std::string struct_name;
    Variable literal_value = Variable(Variable::Type::VOID);
    RuleSet ruleset;

    Parameter() {}
    Parameter(Variable::Type type) : type(type) {}
    Parameter(Variable::Type type, std::string name) : type(type), name(name) {}
    Parameter(Variable::Type type, std::string name, std::string struct_name) : type(type), name(name), struct_name(struct_name) {}
    Parameter(const Parameter& p);
    void operator=(Variable::Type type) {
        this->type = type;
    }

    bool needs_literal_value() const;
    bool needs_function() const;

    bool matches(Variable var) const;
    bool matches(Parameter param) const;

    void operator=(const Parameter& p);

    void set_functiontemplate(Function func);
    Function get_fntemplate() const;
};

bool paramlist_matches(std::vector<Parameter> params1,std::vector<Parameter> params2);
bool ruleset_matches(RuleSet ruleset, Variable value);
bool same_ruleset(RuleSet ra, RuleSet rb);

bool is_ruleset(Token token);
RuleSet construct_ruleset(Token token);

class Function {
public:
    using ReturnType = Parameter;
    std::vector<Line> body;
    std::vector<Parameter> params;
    int parent = -1;
    ReturnType return_type = Variable::Type::UNKNOWN;
    fs::path file;
    // Takes care of the required amount of arguments and their types as well as the return value
    // Throws `MWSMessageException` on error
    Variable run(std::vector<Variable> args, bool method_mode = false);

    std::string to_string() const;

    Function() {}
    Function(const Function& f) {
        this->body = f.body;
        this->parent = f.parent;
        this->file = f.file;
        this->return_type = f.return_type;
        this->params = f.params;
    }

    void operator=(const Function& f) {
        this->body = f.body;
        this->parent = f.parent;
        this->file = f.file;
        this->return_type = f.return_type;
        this->params = f.params;
    }
};

bool is_function_literal(Token token);
std::vector<Function> functions_from_string(std::string src);
std::string funcoverloads_to_string(std::vector<Function> overloads);
Function* function_from_overloads(std::vector<Function>& overloads, std::vector<Variable> args);

bool is_funcparam_literal(Token token);
Function funcparam_from_literal(std::string src);
bool function_match_template(Function templ, Function func);
bool function_match_template(Function templ, std::vector<Function> func);

Parameter returntype_from_string(GeneralTypeToken tkn);

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