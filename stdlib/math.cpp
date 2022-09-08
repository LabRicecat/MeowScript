#include "../inc/meowscript.hpp"
#include <math.h>

MEOWSCRIPT_MODULE

class ModuleMath {
public:
    ModuleMath() {
        Module math("math");
        math.add_command(
            {"max",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                argument_list alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 2) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: max\n\t- Expected: 2\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                auto n1 = tools::check4placeholder(alist[0]);
                auto n2 = tools::check4placeholder(alist[1]);

                if(n1.type != General_type::NUMBER) {
                    throw errors::MWSMessageException{"Wrong argument for argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(n1.type).content,global::get_line()};
                }
                if(n2.type != General_type::NUMBER) {
                    throw errors::MWSMessageException{"Wrong argument for argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(n2.type).content,global::get_line()};
                }

                return std::max(n1.to_variable().storage.number,n2.to_variable().storage.number);
            }}
        ).add_command(
            {"min",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                argument_list alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 2) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: max\n\t- Expected: 2\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                auto n1 = tools::check4placeholder(alist[0]);
                auto n2 = tools::check4placeholder(alist[1]);

                if(n1.type != General_type::NUMBER) {
                    throw errors::MWSMessageException{"Wrong argument for argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(n1.type).content,global::get_line()};
                }
                if(n2.type != General_type::NUMBER) {
                    throw errors::MWSMessageException{"Wrong argument for argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(n2.type).content,global::get_line()};
                }

                return std::min(n1.to_variable().storage.number,n2.to_variable().storage.number);
            }}
        ).add_command(
            {"sqrt",
            {
                car_Number | car_PlaceHolderAble
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                auto v = tools::check4placeholder(args[0]);
                if(v.type != General_type::NUMBER) {
                    throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(v.type).content,global::get_line()};
                }
                return std::sqrt(v.to_variable().storage.number);
            }}
        ).add_command(
            {"floor",
            {
                car_Number | car_PlaceHolderAble
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                auto v = tools::check4placeholder(args[0]);
                if(v.type != General_type::NUMBER) {
                    throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(v.type).content,global::get_line()};
                }
                return std::floor(v.to_variable().storage.number);
            }}
        ).add_command(
            {"ceil",
            {
                car_Number | car_PlaceHolderAble
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                auto v = tools::check4placeholder(args[0]);
                if(v.type != General_type::NUMBER) {
                    throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(v.type).content,global::get_line()};
                }
                return std::ceil(v.to_variable().storage.number);
            }}
        ).add_command(
            {"sin",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                auto alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 1) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: sin\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                auto n = tools::check4placeholder(alist[1]);

                if(n.type != General_type::NUMBER) {
                    throw errors::MWSMessageException{"Wrong argument for argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(n.type).content,global::get_line()};
                }
                return std::sin(args[0].to_variable().storage.number);
            }}
        ).add_command(
            {"cos",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                auto alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 1) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: cos\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                auto n = tools::check4placeholder(alist[1]);

                if(n.type != General_type::NUMBER) {
                    throw errors::MWSMessageException{"Wrong argument for argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(n.type).content,global::get_line()};
                }
                return std::cos(args[0].to_variable().storage.number);
            }}
        ).add_command(
            {"tan",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                auto alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 1) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: tan\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                auto n = tools::check4placeholder(alist[1]);

                if(n.type != General_type::NUMBER) {
                    throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(n.type).content,global::get_line()};
                }
                return std::tan(args[0].to_variable().storage.number);
            }}
        ).add_command(
            {"abs",
            {
                car_Number | car_PlaceHolderAble
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                auto v = tools::check4placeholder(args[0]);
                if(v.type != General_type::NUMBER) {
                    throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(v.type).content,global::get_line()};
                }
                return std::abs(v.to_variable().storage.number);
            }}
        ).add_command(
            {"log10",
            {
                car_Number | car_PlaceHolderAble
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                auto v = tools::check4placeholder(args[0]);
                if(v.type != General_type::NUMBER) {
                    throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(v.type).content,global::get_line()};
                }
                return std::log10(v.to_variable().storage.number);
            }}
        );
        
        math.enabled = false;
        add_module(math);
    }
}static module_math;