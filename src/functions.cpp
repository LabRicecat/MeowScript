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

Parameter::Parameter(const Parameter& p) {
    this->type = p.type;
    this->name = p.name;
    this->struct_name = p.struct_name;
    this->literal_value = p.literal_value;
    if(p.m_fntmpl != nullptr) {
        if(this->m_fntmpl == nullptr) this->m_fntmpl = std::make_unique<Function>();
        *this->m_fntmpl = *p.m_fntmpl;
    }
}

void Parameter::operator=(const Parameter& p) {
    this->type = p.type;
    this->name = p.name;
    this->struct_name = p.struct_name;
    this->literal_value = p.literal_value;
}

void Parameter::set_functiontemplate(Function func) {
    if(m_fntmpl != nullptr) m_fntmpl.reset();
    m_fntmpl = std::make_unique<Function>();
    *m_fntmpl = func;
}

bool Parameter::needs_function() const {
    return m_fntmpl != nullptr;
}

Function Parameter::get_fntemplate() const {
    return *m_fntmpl;
}

bool Parameter::needs_literal_value() const {
    return literal_value.type != Variable::Type::VOID;
}

bool Parameter::matches(Variable var) const {
    if(needs_literal_value() && var != literal_value) return false;
    if(!ruleset_matches(ruleset,var)) return false;
    if(needs_function() && var.type != Variable::Type::Function) return false;
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
            needs_function() &&
            function_match_template(get_fntemplate(),var.storage.func)
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
    if(needs_function() != param.needs_function()) return false;
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
            needs_function() &&
            function_match_template(get_fntemplate(),param.get_fntemplate())
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
    if(parent >= 0) load_scope(parent,{},true);
    gtt_ret = run_lexed(body,false,true,-1,arg_map,this->file);
    if(parent >= 0) pop_scope();
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

std::string Function::to_string() const {
    std::string ret = "[Function ";
    ret += "(";
    for(auto i : params) {
        if(i.needs_literal_value())
            ret += i.literal_value.to_string();
        else ret += i.name;
        
        if(!i.ruleset.empty()) {
            ret += " (";
            for(auto j : i.ruleset) {
                ret += j.op_name + " " + j.value.to_string() + ",";
            }
            ret.pop_back();
            ret += ")";
        }
        if(i.type != Variable::Type::UNKNOWN && i.type != Variable::Type::ANY) {
            ret += " :: " + var_t2token(i.type).content;
        }

        ret += ",";
    }
    if(!params.empty()) {
        ret.pop_back();
    }
    ret += ")";

    ret += " -> ";
    if(return_type.type == Variable::Type::UNKNOWN) ret += "Any";
    else ret += var_t2token(return_type.type).content;

    ret += " {";
    for(auto i : body) {
        for(auto j : i.source) {
            ret += j.content + " ";
        }
        if(!i.source.empty()) ret.pop_back();
        ret += "; ";
    }
    if(!body.empty()) {ret.pop_back();ret.pop_back();}
    ret += "}]";
    return ret;
}

#define CHECK4(idx,car) if(!car.matches(get_type(tokens[idx],car))) return false;

bool MeowScript::is_function_literal(Token token) {
    if(token.in_quotes || token.content == "") return false;
    if(!brace_check(token,'[',']')) return false;
    token.content.erase(token.content.begin());
    token.content.erase(token.content.end()-1);

    auto l = lex_text(token.content);
    std::vector<Token> tokens;
    for(auto i : l)
        for(auto j : i.source)
            tokens.push_back(j);
    
    if(tokens.size() != 5) return false;

    if(tokens[0].content != "Function") return false;
    CHECK4(1,car_ParameterList)
    CHECK4(2,car_Operator)
    if(tokens[2].content != "->") return false;
    if(!is_valid_function_return(tokens[3].content)) return false;
    CHECK4(4,car_Compound)

    return true;
}

// #undef CHECK4 <-- later

std::vector<Function> MeowScript::functions_from_string(std::string src) {
    if(!is_function_literal(src)) return std::vector<Function>{};
    std::vector<Function> ret;
    src.pop_back();
    src.erase(src.begin());
    auto l = lex_text(src);
    std::vector<Token> tokens;
    for(auto i : l)
        for(auto j : i.source)
            tokens.push_back(j);

    tokens.erase(tokens.begin());

    Command* command = get_command("func"); // first overload is the biggest one
    if(command == nullptr) {
        // eh
    }

    std::vector<GeneralTypeToken> gtts;
    std::vector<Token> tks;
    for(size_t i = 0; i < tokens.size(); ++i) {
        while(i < tokens.size() && tokens[i].content != "|") {
            tks.push_back(tokens[i]);
            ++i;
        }
        tks.insert(tks.begin(),"__F");
        if(tks.size() != command->args.size()) {
            // error
        }
        for(size_t i = 0; i < tks.size(); ++i) {
            gtts.push_back(GeneralTypeToken{tks[i],command->args[i]});
        }
        new_scope();
        command->run(gtts);
        Function f = current_scope()->functions["__F"][0];
        pop_scope();
        ret.push_back(f);
        ++i; // the "|"
    }
    
    return ret;
}

std::string MeowScript::funcoverloads_to_string(std::vector<Function> overloads) {
    if(overloads.empty()) return "";
    std::string ret = "[Function ";
    for(auto i : overloads) {
        std::string tmp = i.to_string();
        tmp.erase(tmp.begin(),tmp.begin() + ((std::string)"[Function ").size());
        tmp.pop_back();
        ret += tmp + " | ";
    }
    ret.pop_back();ret.pop_back();ret.pop_back();
    ret += "]";
    return ret;
}

Function* MeowScript::function_from_overloads(std::vector<Function>& overloads, std::vector<Variable> args) {
    for(auto& i : overloads) {
        if(func_param_match(i,args)) {
            return &i;
        }
    }
    return nullptr;
}

bool MeowScript::is_funcparam_literal(Token token) {
    if(token.in_quotes || token.content == "") return false;
    if(!brace_check(token,'[',']')) return false;
    token.content.erase(token.content.begin());
    token.content.erase(token.content.end()-1);

    auto l = lex_text(token.content);
    std::vector<Token> tokens;
    for(auto i : l)
        for(auto j : i.source)
            tokens.push_back(j);
    
    if(tokens.size() != 4) return false;

    if(tokens[0].content != "Function") return false;
    CHECK4(1,car_ParameterList)
    CHECK4(2,car_Operator)
    if(tokens[2].content != "->") return false;
    if(!is_valid_function_return(tokens[3].content)) return false;

    return true;
}

#undef CHECK4

Function MeowScript::funcparam_from_literal(std::string src) {
    if(!is_funcparam_literal(src)) return Function{};
    Function ret;
    src.erase(src.begin());
    src.erase(src.end()-1);
    src += " {}";
    src.erase(src.begin(),src.begin() + ((std::string)"[Function").size());
    src = "__F " + src;
    auto l = lex_text(src);
    std::vector<Token> tks;
    for(auto i : l)
        for(auto j : i.source)
            tks.push_back(j);

    Command* command = get_command("func");
    std::vector<GeneralTypeToken> gtts;
    for(size_t i = 0; i < tks.size(); ++i) {
        gtts.push_back(GeneralTypeToken{tks[i],command->args[i]});
    }
    new_scope();
    command->run(gtts);
    Function f = current_scope()->functions["__F"][0];
    pop_scope();

    ret.params = f.params;
    ret.return_type = f.return_type;
    return ret;
}

bool MeowScript::function_match_template(Function templ, Function func) {
    return \
        templ.return_type.matches(func.return_type) &&
        paramlist_matches(templ.params,func.params);
}

bool MeowScript::function_match_template(Function templ, std::vector<Function> func) {
    for(auto i : func) {
        if(function_match_template(templ,i)) {
            return true;
        }
    }
    return false;
}

Parameter MeowScript::returntype_from_string(GeneralTypeToken tkn) {
    Parameter return_type;
    if(tkn.source.content == "Any") {
        return_type = Variable::Type::ANY;
    }
    else if(tkn.source.content == "Void") {
        return_type = Variable::Type::VOID;
    }
    else if(tkn.type == General_type::STRUCT) {
        return_type.type = Variable::Type::Object;
        return_type.struct_name = tkn.source.content;
    }
    else if(tkn.source.content == "Object") {
        return_type.type = Variable::Type::Object;
        return_type.struct_name = "";
    }
    else if(is_funcparam_literal(tkn.source.content)) {
        return_type.set_functiontemplate(funcparam_from_literal(tkn.source.content));
        return_type.type = Variable::Type::Function; 
    }
    else {
        return_type = token2var_t(tkn.to_string());
    }
    return return_type;
}