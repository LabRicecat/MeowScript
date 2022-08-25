cmake_minimum_required(VERSION 3.9)
project(meow-script 
    VERSION 0.1.0 
    LANGUAGES CXX
    DESCRIPTION "An easy extendable scripting language"
)

set(BINARY meow-script)
set(SOURCE 
    src/commands.cpp
    src/expressions.cpp 
    src/functions.cpp 
    src/global.cpp 
    src/main.cpp 
    src/modules.cpp 
    src/patterns.cpp 
    src/reader.cpp
    src/runner.cpp
    src/scopes.cpp
    src/tools.cpp 
    src/variables.cpp 

    stdlib/filesystem.cpp 
    stdlib/math.cpp 
    stdlib/os.cpp 
    )

add_executable(${BINARY} ${SOURCE})

# can be ignored if it errors
if(UNIX)
target_compile_options(${BINARY} PRIVATE -g)
endif
