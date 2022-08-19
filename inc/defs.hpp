#ifndef MEOWSCRIPT_IG_DEFS_HPP
#define MEOWSCRIPT_IG_DEFS_HPP

#define MEOWSCRIPT_VERSION_PATCH 0
#define MEOWSCRIPT_VERSION_MINOR 0
#define MEOWSCRIPT_VERSION_MAJOR 1

#define MEOWSCRIPT_VERSION_STR std::to_string(MEOWSCRIPT_VERSION_PATCH) + "." + std::to_string(MEOWSCRIPT_VERSION_MINOR) + "." + std::to_string(MEOWSCRIPT_VERSION_MAJOR)

#define MEOWSCRIPT_HEADER_BEGIN namespace MeowScript {
#define MEOWSCRIPT_HEADER_END }

#define MEOWSCRIPT_SOURCE_FILE using namespace MeowScript;

#define MEOWSCRIPT_MODULE MEOWSCRIPT_SOURCE_FILE

#ifdef __linux__
# define MEOWSCRIPT_DIR_SL "/"
# define MEOWSCRIPT_USE_LINUX
# define MEOWSCRIPT_SHARED_OBJECT_EXT ".so"
# define MEOWSCRIPT_OS_NAME "Linux"
#elif defined(_WIN32)
# define MEOWSCRIPT_DIR_SL "\\"
# define MEOWSCRIPT_USE_WINDOWS
# define MEOWSCRIPT_SHARED_OBJECT_EXT ".dll"
# define MEOWSCRIPT_OS_NAME "Windows"
#else
# define MEOWSCRIPT_DIR_SL "/" // something like that?
# define MEOWSCRIPT_USE_ELSE
# define MEOWSCRIPT_SHARED_OBJECT_EXT ""
# define MEOWSCRIPT_OS_NAME "Unknown"
#endif

#include <filesystem>
namespace fs = std::filesystem;

#endif