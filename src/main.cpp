#include "../inc/runner.hpp"
#include "../inc/tools.hpp"
#include "../inc/expressions.hpp"
#include "../inc/modules.hpp"

void print_help() {
    std::cout << "## MeowScript ##\n" 
              << "A dynamic and easy extendable programming language.\n\n"
              << "Usage: meow-script [options] [file]\n"
              << "Options are:\n"
              << " --help, -h \t: Prints this and exits\n"
              << " --version, -v \t: Prints the version and exits\n"
              << " --shell, -s \t: Opens the MeowScript shell\n\n"
              << "(c) SirWolf 2022\n"
              << "Github repo: https://github.com/SirWolfi/MeowScript\n";
}

void error_message_pretty(MeowScript::errors::MWSMessageException& err) {
    std::cout << "\n--------\nError in line " << err.get_line() << ":\n" << err.what() << "\n";
    std::cout << "\nCall Trace (most recent first)\n";

    while(!MeowScript::global::call_trace.empty()) {
        auto [line_c,line_s,file] = MeowScript::global::call_trace.top();
        MeowScript::global::pop_trace();

        std::cout << "In " << file << ":" << line_c << "\n- " << line_s << "\n";
    }
}

int main(int argc, char** argv) {
    if(argc == 1) {
        print_help();
        return 0;
    }

    std::string arg = std::string(argv[1]);

    std::vector<std::string> args;
    for(size_t i = 2; i < argc; ++i) {
        args.push_back(std::string(argv[i]));
    }

    if(!MeowScript::check_paths()) {
        MeowScript::make_paths();
    }

    if(arg == "--help" || arg == "-h") {
        print_help();
        return 0;
    }
    else if(arg == "--version" || arg == "-v") {
        std::cout << MEOWSCRIPT_VERSION_STR << "\n";
        return 0;
    }
    else if(arg == "--shell" || arg == "-s") {
        MeowScript::new_scope();
        std::string input;
        std::cout << ">>> MeowScript shell <<<\n"
            << "Type `exit` to exit the shell.\n";
        while(true) {
            std::cout << "$> ";
            std::getline(std::cin,input);
            if(input == "exit") {
                MeowScript::pop_scope();
                return 0;
            }
            try {
                MeowScript::run_text(input,false,false,-1,{},std::filesystem::current_path(),false,true);
            }
            catch(MeowScript::errors::MWSMessageException& err) {
                error_message_pretty(err);
                return 1;
            }
        }

        MeowScript::pop_scope();
        return 0;
    }
    else {
        if(!MeowScript::check_paths()) {
            MeowScript::make_paths();
        }

        if(!std::filesystem::exists(std::string(argv[1]))) {
            std::cout << "No such file: \"" + std::string(argv[1]) + "\"\n";
            return 1;
        }
        else if(std::filesystem::is_directory(std::string(argv[1]))) {
            std::cout << "Can't parse directory! (\"" + std::string(argv[1]) + "\")\n";
            return 1;
        }
        try {
            for(auto i : args) {
                MeowScript::Variable var;
                var.storage.string = i;
                var.storage.string.in_quotes = true;
                var.type = MeowScript::Variable::Type::String;
                MeowScript::global::args.push_back(var);
            }
            std::filesystem::path from = std::filesystem::path(std::string(argv[1]));
            MeowScript::global::origin_file = from;
            MeowScript::run_file(std::string(argv[1]),true,false,-1,{},from);
        }
        catch(MeowScript::errors::MWSMessageException& err) {
            error_message_pretty(err);
            return 1;
        }
        return 0;
    }

    return 1;
}