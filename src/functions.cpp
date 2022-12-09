#include "../inc/functions.hpp"
#include "../inc/global.hpp"
#include "../inc/runner.hpp"
#include "../inc/objects.hpp"
#include "../inc/expressions.hpp"

MEOWSCRIPT_SOURCE_FILE

bool ArgRule::applies(Variable var) const {
    auto r = (*operat).parse(var,value);
    if(r.type != Variable::Type::Number) return false;
    return r.storage.number == 1;
}
bool ArgRule::operator==(ArgRule r) {
    return *operat == *r.operat && value == r.value && op_name == r.op_name;
}

bool Parameter::needs_literal_value() const {
    return literal_value.type != Variable::Type::VOID;
}

bool Parameter::matches(Variable var) const {
    if(needs_literal_value() && var != literal_value) return false;
    if(!ruleset_matches(ruleset,var)) return false;
    return \
        (
            needs_literal_value() &&
            var == literal_value
        ) ||
        type == Variable::Type::UNKNOWN ||
        type == Variable::Type::ANY ||
        (
            var.type == Variable::Type::Object &&
            struct_name == "" &&
            type == Variable::Type::Object
        ) ||
        (
            var.type == Variable::Type::Object &&
            type == Variable::Type::Object &&
            struct_matches(get_struct(struct_name),&var.storage.obj)
        ) ||
        (
            var.type != Variable::Type::Object &&
            var.type == type
        );
}
// TODO: fix potential recursion problem
bool Parameter::matches(Parameter param) const {
    if(param.needs_literal_value() != needs_literal_value()
    || (param.needs_literal_value() && param.literal_value != literal_value)) return false;
    if(!same_ruleset(ruleset,param.ruleset)) return false;
    return \
        (
            needs_literal_value() &&
            param.needs_literal_value() && 
            param.literal_value == literal_value
        ) ||
        type == Variable::Type::UNKNOWN ||
        type == Variable::Type::ANY ||
        (
            param.type == Variable::Type::Object &&
            struct_name == "" &&
            type == Variable::Type::Object
        ) ||
        (
            param.type == Variable::Type::Object &&
            param.struct_name == "" &&
            type == Variable::Type::Object
        ) ||
        (
            param.type == Variable::Type::Object &&
            type == Variable::Type::Object &&
            struct_matches(get_struct(struct_name),get_struct(param.struct_name))
        ) ||
        (
            param.type != Variable::Type::Object &&
            param.type == type
        );
}

bool MeowScript::paramlist_matches(std::vector<Parameter> params1,std::vector<Parameter> params2) {
    for(size_t i = 0; i < params1.size(); ++i) {
        if(!params1[i].matches(params2[i])) {
            return false;
        }
    }
    return true;
}

bool MeowScript::ruleset_matches(RuleSet ruleset, Variable value) {
    for(auto i : ruleset) {
        if(!i.applies(value)) return false;
    }
    return true;
}

bool MeowScript::same_ruleset(RuleSet ra, RuleSet rb) {
    if(ra.size() != rb.size()) return false;
    for(size_t i = 0; i < ra.size(); ++i) {
        bool f = false;
        for(size_t j = 0; j < rb.size(); ++j) {
            if(ra[i] == rb[j]) {
                f = true;
                break;
            }
        }
        if(!f) return false;
    }
    return true;
}

bool MeowScript::is_ruleset(Token token) {
    if(token == "" || token.in_quotes) return false;
    if(!brace_check(token,'(',')')) return false;

    token.content.erase(token.content.begin());
    token.content.erase(token.content.end()-1);
    lexed_tokens l = lex_text(token.content);
    std::vector<Token> line;
    for(auto i : l)
        for(auto j : i.source)
            line.push_back(j);

    ArgRule tmp;
    for(size_t i = 0; i < line.size(); ++i) {
        if(is_valid_operator_name(line[i])) {
            if(tmp.op_name != "") {
                return false;
            }
            else {
                if(i+1 == line.size()) {
                    return false;
                }
                tmp.op_name = line[i];
                ++i;
            }
        }
        else {
            if(line[i].content != ",") {
                return false;
            }
            tmp.op_name = "";
        }
    }
    return true;
}

RuleSet MeowScript::construct_ruleset(Token token) {
    if(token == "" || token.in_quotes) return RuleSet();
    if(!brace_check(token,'(',')')) return RuleSet();

    token.content.erase(token.content.begin());
    token.content.erase(token.content.end()-1);
    lexed_tokens l = lex_text(token.content);
    std::vector<Token> line;
    for(auto i : l)
        for(auto j : i.source)
            line.push_back(j);
    
    RuleSet ret;
    ArgRule tmp;
    for(size_t i = 0; i < line.size(); ++i) {
        if(is_valid_operator_name(line[i])) {
            if(tmp.op_name != "") {
                throw errors::MWSMessageException{"Invalid ruleset syntax!",global::get_line()};
            }
            else {
                if(i+1 == line.size()) {
                    throw errors::MWSMessageException{"Expected value after operator in ruleset!",global::get_line()};
                }
                else {
                    Token literal = line[i+1];
                    tmp.value = make_variable(literal);
                }
                Operator* o = get_operator(line[i],General_type::UNKNOWN,var_t2general_t(tmp.value.type));
                if(o == nullptr) {
                    throw errors::MWSMessageException{"Unknown operator!",global::get_line()};
                }
                tmp.operat = o;
                tmp.op_name = line[i];
                ++i;
            }
        }
        else {
            if(line[i].content != ",") {
                throw errors::MWSMessageException{"Invalid ruleset syntax!",global::get_line()};
            }
            ret.push_back(tmp);
            tmp.op_name = "";
            tmp.operat = nullptr;
            tmp.value = Variable(Variable::Type::VOID);
        }
    }
    if(tmp.op_name != "") {
        ret.push_back(tmp);
    }
    return ret;
}

Variable MeowScript::Function::run(std::vector<Variable> args, bool method_mode) {
    if(args.size() != this->params.size()) {
        std::string err = "Too many/few arguemnts for function!\n\t- Expected: " + std::to_string(this->params.size()) + "\n\t- But got: " + std::to_string(args.size());
        throw errors::MWSMessageException{err,global::get_line()};
    }
    
    std::map<std::string,Variable> arg_map;
    for(size_t i = 0; i < args.size(); ++i) {
        if(!this->params[i].matches(args[i])) {
            std::string err = "Invalid argument! (" + std::to_string(i) + ")\n\t- Expected: " + var_t2token(this->params[i].type).content + "\n\t- But got: " + var_t2token(args[i].type).content;
            throw errors::MWSMessageException{err,global::get_line()};
        }

        arg_map[params[i].name] = args[i]; 
    }
    int saved_istruct = global::in_struct;
    global::in_struct = 0;
    GeneralTypeToken gtt_ret;
    if(method_mode) {
        gtt_ret = run_lexed(body,false,false,-1,arg_map,this->file);
    }
    else {
        gtt_ret = run_lexed(body,false,true,scope_idx,arg_map,this->file);
    }
    global::in_struct = saved_istruct;
    if(gtt_ret.type != General_type::VOID && gtt_ret.type != General_type::UNKNOWN) {
        Variable var_ret;
        try {
            var_ret = gtt_ret.to_variable();
        }
        catch(errors::MWSMessageException& err) {
            std::string err_msg = "Invalid return type!\n\t- Expected: " + var_t2token(return_type.type).content + "\n\t- But got: " + general_t2token(gtt_ret.type).content;
            throw errors::MWSMessageException{err_msg,global::get_line()};
        }
        return var_ret;
    }
    else {
        return Variable();
    }
}