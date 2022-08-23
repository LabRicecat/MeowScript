#ifndef MEOWSCRIPT_IG_COMMANDS_HPP
#define MEOWSCRIPT_IG_COMMANDS_HPP

#include "defs.hpp"
#include "errors.hpp"
#include "reader.hpp"

MEOWSCRIPT_HEADER_BEGIN

struct CommandArgReqirement {
    std::vector<int> carry;

    CommandArgReqirement operator|(CommandArgReqirement car) {
        CommandArgReqirement car_c = car;
        CommandArgReqirement this_c = *this;
        for(auto i : car_c.carry) {
            this_c.carry.push_back(i);
        }
        this_c.single = false;
        return this_c;
    }
    // Do not modify!
    bool single = true;

    void operator=(int i) {
        carry.clear();
        single = true;
        carry.push_back(i);
    }
    void operator=(General_type type) {
        carry.clear();
        single = true;
        carry.push_back(static_cast<int>(type)+1);
    }

    bool operator==(CommandArgReqirement car) {
        if(!car.single || !single || car.carry.empty() || carry.empty()) {
            return false;
        }
        return carry[0] == car.carry[0];
    }

    CommandArgReqirement(int i) {
        operator=(i);
    }
    CommandArgReqirement(General_type i) {
        operator=(i);
    }
    CommandArgReqirement() {}

    bool has_carry(int in) const;
    bool matches(Token tk);
    bool matches(CommandArgReqirement car);
    bool matches(General_type type);
};

inline CommandArgReqirement car_Any = 0;
inline CommandArgReqirement car_Number = General_type::NUMBER;
inline CommandArgReqirement car_String = General_type::STRING;
inline CommandArgReqirement car_List = General_type::LIST;
inline CommandArgReqirement car_Function = General_type::FUNCTION;
inline CommandArgReqirement car_Command = General_type::COMMAND;
inline CommandArgReqirement car_Compound = General_type::COMPOUND;
inline CommandArgReqirement car_Name = General_type::NAME;
inline CommandArgReqirement car_Expression = General_type::EXPRESSION;
inline CommandArgReqirement car_ArgumentList = General_type::ARGUMENTLIST;
inline CommandArgReqirement car_Operator = General_type::OPERATOR;
inline CommandArgReqirement car_Module = General_type::MODULE;
inline CommandArgReqirement car_Event = General_type::EVENT;
inline CommandArgReqirement car_Keyword = General_type::KEYWORD;
inline CommandArgReqirement car_PlaceHolderAble = car_Expression | car_Compound | car_Name;


struct GeneralTypeToken;

struct Command {
    std::string name;
    std::vector<CommandArgReqirement> args;

    GeneralTypeToken (*run)(std::vector<GeneralTypeToken> args);
};

std::vector<Command>* get_command_list();

bool is_command(std::string name);
Command* get_command(std::string name);

MEOWSCRIPT_HEADER_END

#endif