#include "../inc/runner.hpp"
#include "../inc/tools.hpp"
#include "../inc/expressions.hpp"
#include "../inc/modules.hpp"

void print_help() {

}

int main(int argc, char** argv) {
    if(argc == 1) {
        print_help();
        return 0;
    }

    std::string arg = std::string(argv[1]);

    if(arg == "--help" || arg == "-h") {
        print_help();
        return 0;
    }
    else if(arg == "--version" || arg == "-v") {
        std::cout << MEOWSCRIPT_VERSION_STR << "\n";
        return 0;
    }
    else {
        if(!MeowScript::check_paths()) {
            MeowScript::make_paths();
        }
        try {
            std::filesystem::path from = std::filesystem::path(std::string(argv[1]));
            MeowScript::run_file(std::string(argv[1]),true,false,-1,{},from);
        }
        catch(MeowScript::errors::MWSMessageException& err) {
            std::cout << "Error in line " << err.get_line() << ":\n" << err.what() << "\n";
            return 1;
        }
        return 0;
    }

    return 1;
}