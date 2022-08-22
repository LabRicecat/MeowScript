#include "../inc/meowscript.hpp"
#include <math.h>
#include <random>
#include <thread>

#ifdef MEOWSCIPT_USE_WINDOWS
#include <windows.h>

void sleep_(int milliseconds) {
    Sleep(milliseconds);
}
#elif defined(MEOWSCRIPT_USE_LINUX)

void sleep_(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
#endif

MEOWSCRIPT_MODULE

class ModuleOS {
public:
    ModuleOS() {
        Module os("os");
        os.add_command(
            {"rand",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                argument_list alist = tools::parse_argument_list(args[0]);
                if(alist.size() > 2) {
                    throw errors::MWSMessageException{"Too many arguments for command: rand\n\t- Expected: <=2\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                int max = 0;
                int min = 0;
                
                if(alist.size() > 0 && alist[0].type == General_type::NUMBER) {
                    max = alist[0].to_variable().storage.number;
                }
                else if(alist.size() > 0) {
                    throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(alist[0].type).content,global::get_line()};
                }

                if(alist.size() > 1 && alist[1].type == General_type::NUMBER) {
                    min = alist[1].to_variable().storage.number;
                }
                else if(alist.size() > 1) {
                    throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(alist[0].type).content,global::get_line()};
                }

                if(min > max) {
                    throw errors::MWSMessageException{"Invalid argument! \"max\" is not allowed to be smaller than \"min\"!",global::get_line()};
                }
                srand(time(NULL)+rand()*rand()-rand());
                int result = 0;
                if(max == min) {
                    return max;
                }
                if(max == 0)
                    return (rand() + min);
                result = (rand() % (max-min) + min);
                if(result > max) {
                    result = max;
                }
                return result;
                
            }}
        ).add_command(
            {"date",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                argument_list alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 0) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: date\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                time_t t;
                time(&t);
                std::string ret = std::asctime(std::localtime(&t));
                return ret.substr(0,ret.size()-1); // removes a nasty '\n'
            }}
        ).add_command(
            {"sleep",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                argument_list alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 1) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: sleep\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                alist[0] = tools::check4placeholder(alist[0]);
                if(alist[0].type != General_type::NUMBER) {
                    throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(alist[0].type).content,global::get_line()};
                }
                
                sleep_(alist[0].to_variable().storage.number);
                return general_null;
            }}
        ).add_command(
            {"name",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                argument_list alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 0) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: name\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                return MEOWSCRIPT_OS_NAME;
            }}
        ).add_command(
            {"exit",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                argument_list alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 1) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: exit\n\t- Expected: 1\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                alist[0] = tools::check4placeholder(alist[0]);
                if(alist[0].type != General_type::NUMBER) {
                    throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(alist[0].type).content,global::get_line()};
                }
                std::exit(alist[0].to_variable().storage.number);
                return general_null;
            }}
        );
        os.enabled = false;
        add_module(os);
    }
}static module_os;