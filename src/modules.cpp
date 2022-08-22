#include "../inc/modules.hpp"
#include "../inc/commands.hpp"
#include "../inc/variables.hpp"
#include "../inc/global.hpp"

#include "../inc/meowscript.hpp"

MEOWSCRIPT_SOURCE_FILE

#include <filesystem>
#include <fstream>

#ifdef MEOWSCRIPT_USE_LINUX
#include <pwd.h>
#include <unistd.h>
#include <dlfcn.h>

std::string MeowScript::get_username() {
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw)
    {
        return std::string(pw->pw_name);
    }
    return {};
}

#define MEOWSCRIPT_PATH ("/home/" + MeowScript::get_username() + "/.meowscript/")
#define MEOWSCRIPT_CONFIG_PATH ("/home/" + MeowScript::get_username() + "/.meowscript/config.mwsconf")
#define MEOWSCRIPT_MODULE_PATH ("/home/" + MeowScript::get_username() + "/.meowscript/lib/")

void MeowScript::load_all_modules() {
    for(auto i : std::filesystem::recursive_directory_iterator(MEOWSCRIPT_MODULE_PATH)) {
        if(i.path().extension() == MEOWSCRIPT_SHARED_OBJECT_EXT) {
            std::string pth = i.path().string();
            void* handle = dlopen(pth.c_str(),RTLD_LAZY);
            if(handle == nullptr) {
                throw errors::MWSMessageException{dlerror(),0};
            }
        }
    }
}

bool MeowScript::load_module(std::string name) {
    for(auto i : std::filesystem::recursive_directory_iterator(MEOWSCRIPT_MODULE_PATH)) {
        if(i.path().filename() == name && i.path().extension() == MEOWSCRIPT_SHARED_OBJECT_EXT) {
            std::string pth = i.path().string();
            void* handle = dlopen(pth.c_str(),RTLD_LAZY);
            if(handle == nullptr) {
                throw errors::MWSMessageException{dlerror(),0};
            }
            return true;
        }
    }
    return false;
}

#elif defined(MEOWSCRIPT_USE_WINDOWS)
#include <windows.h>
#include <Lmcons.h>

#define MEOWSCRIPT_PATH "C:\\ProgramData\\.meowscript\\"
#define MEOWSCRIPT_CONFIG_PATH "C:\\ProgramData\\.meowscript\\config.mwsconf"
#define MEOWSCRIPT_MODULE_PATH "C:\\ProgramData\\.meowscript\\lib\\"

static std::string wstring2string(std::wstring wstr) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX,wchar_t> converterX;

    return converterX.to_bytes(wstr);
}
static std::wstring string2wstring(std::string str) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX,wchar_t> converterX;

    return converterX.to_bytes(str);
}

std::string MeowScript::get_username() {
    TCHAR username[UNLEN+1];
    DWORD size = UNLEN +1;
    GetUserName((TCHAR*)username,&size);

    std::wstring tmp(username,size-1);
    
    return wstring2string(tmp);
}

void MeowScript::load_all_modules() {
    for(auto i : std::filesystem::recursive_directory_iterator(MEOWSCRIPT_MODULE_PATH)) {
        if(i.path().extension() == ".dll") {
            auto pth = string2wstring(i.path().string());
            auto h = LoadLibrary(pth.c_str());
            if(h) {
                throw errors::MWSMessageException{"Error while loading module!",0};
            }
        }
    }
}

bool MeowScript::load_module(std::string name) {
    for(auto i : std::filesystem::recursive_directory_iterator(MEOWSCRIPT_MODULE_PATH)) {
        if(i.path().filename() == name) {
            auto pth = string2wstring(i.path().string());
            auto h = LoadLibrary(pth.c_str());
            if(h) {
                throw errors::MWSMessageException{"Error while loading module!",0};
            }
            return true;
        }
    }
    return false;
}

#else

std::string MeowScript::get_username() {
    // meh...
}

#define MEOWSCRIPT_PATH ""
#define MEOWSCRIPT_CONFIG_PATH ""
#define MEOWSCRIPT_MODULE_PATH ""

#endif

Command* MeowScript::Module::get_command(std::string name) {
    for(auto& i : commands) {
        if(i.name == name) {
            return &i;
        }
    }
    return nullptr;
}

bool MeowScript::Module::has_command(std::string name) {
    return this->get_command(name) != nullptr;
}

Variable* MeowScript::Module::get_variable(std::string name) {
    if(has_variable(name)) {
        return &this->variables[name];
    }
    return nullptr;
}

bool MeowScript::Module::has_variable(std::string name) {
    return this->variables.count(name) != 0;
}

Module& MeowScript::Module::add_command(Command command) {
    commands.push_back(command);
    return *this;
}

Module& MeowScript::Module::add_variable(std::string name, Variable var) {
    variables[name] = var;
    return *this;
}

Module* MeowScript::get_module(std::string name) {
    for(auto& i : modules) {
        if(i.name == name) {
            return &i;
        }
    }
    return nullptr;
}

bool MeowScript::is_loaded_module(std::string name) {
    return get_module(name) != nullptr;
}

void MeowScript::add_module(Module module) {
    if(!is_loaded_module(module.name)) {
        modules.push_back(module);
    }
    else {
        throw errors::MWSMessageException{"Name conflict! Tried to add module \"" + module.name + "\" twice!",global::get_line()};
    }
}

std::vector<Module>* MeowScript::get_module_list() {
    return &modules;
}

bool MeowScript::is_loadable_module(std::string name) {
    for(auto i : std::filesystem::recursive_directory_iterator(MEOWSCRIPT_MODULE_PATH)) {
        if(i.path().extension() == MEOWSCRIPT_SHARED_OBJECT_EXT && i.path().filename() == name + MEOWSCRIPT_SHARED_OBJECT_EXT) {
            return true;
        }
    }
    return false;
}

void MeowScript::make_paths() {
    if(!std::filesystem::exists(MEOWSCRIPT_PATH)) {
        std::filesystem::create_directory(MEOWSCRIPT_PATH);
    }
    if(!std::filesystem::exists(MEOWSCRIPT_CONFIG_PATH)) {
        std::ofstream of(MEOWSCRIPT_CONFIG_PATH, std::ios::app);
        of.close();
    }
    if(!std::filesystem::exists(MEOWSCRIPT_MODULE_PATH)) {
        std::filesystem::create_directory(MEOWSCRIPT_MODULE_PATH);
    }
}

bool MeowScript::check_paths() {
    if(!std::filesystem::exists(MEOWSCRIPT_PATH)) {
        return false;
    }
    if(!std::filesystem::exists(MEOWSCRIPT_CONFIG_PATH)) {
        return false;
    }
    if(!std::filesystem::exists(MEOWSCRIPT_MODULE_PATH)) {
        return false;
    }
    return true;
}

bool MeowScript::build_stdlib() {
    if(!std::filesystem::exists(".." MEOWSCRIPT_DIR_SL "stdlib")) {
        std::cout << "Could not find stdlib/ directory! Sure you run in installation folder?\n";
        return false;
    }
    if(!system(NULL)) {
        std::cout << "No shell avalable, try to execute meow-script somehow else!\n";
    }
    system("cd .. && cd stdlib && mkdir build && cd build && cmake ../ && make");
    for(auto i : std::filesystem::recursive_directory_iterator(".." MEOWSCRIPT_DIR_SL "stdlib" MEOWSCRIPT_DIR_SL "build")) {
        if(i.path().extension() == MEOWSCRIPT_SHARED_OBJECT_EXT) {
            std::string name = i.path().filename().string();
            name.erase(name.begin(),name.begin()+3); // remove the `lib`
            
            if(std::filesystem::exists(std::string(MEOWSCRIPT_MODULE_PATH) + name.c_str())) {
                std::filesystem::remove_all(std::string(MEOWSCRIPT_MODULE_PATH) + name.c_str());
            }
            std::filesystem::copy(i.path(),MEOWSCRIPT_MODULE_PATH + name);
        }
    }
    std::filesystem::remove_all(".." MEOWSCRIPT_DIR_SL "stdlib" MEOWSCRIPT_DIR_SL "build");
    return true;
}