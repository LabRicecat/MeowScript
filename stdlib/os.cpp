#include "../inc/meowscript.hpp"
#include <math.h>
#include <random>
#include <thread>

#ifdef MEOWSCIPT_USE_WINDOWS
#include <windows.h>

void mw_sleep(int milliseconds) {
    Sleep(milliseconds);
}

void mw_clear() {
    system("cls");
}
#elif defined(MEOWSCRIPT_USE_LINUX)
#include <stdlib.h>

void mw_sleep(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void mw_clear() {
    system("clear");
}
#else
void mw_sleep(int milliseconds) {}
void mw_clear() {}
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
                
                mw_sleep(alist[0].to_variable().storage.number);
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
        ).add_command(
            {"version",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                argument_list alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 0) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: version\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                return MEOWSCRIPT_VERSION_STR;
            }}
        ).add_command(
            {"origin_file",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                argument_list alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 0) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: origin_file\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                return global::origin_file;
            }}
        ).add_command(
            {"main_file",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                argument_list alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 0) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: main_file\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                return global::origin_file == global::include_path.top().string();
            }}
        ).add_command(
            {"file",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                argument_list alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 0) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: file\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                return global::include_path.top().string();
            }}
        ).add_command(
            {"clear",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                argument_list alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 0) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: clear\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                mw_clear();
                return general_null;
            }}
        ).add_command(
            {"args",
            {
                car_ArgumentList
            },
            [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                argument_list alist = tools::parse_argument_list(args[0]);
                if(alist.size() != 0) {
                    throw errors::MWSMessageException{"Too many/few arguments for command: args\n\t- Expected: 0\n\t- But got: " + std::to_string(alist.size()) ,global::get_line()};
                }
                List ret;
                ret.elements = global::args;
                return ret;
            }}
        );
        os.enabled = false;
        add_module(os);
    }
}static module_os;