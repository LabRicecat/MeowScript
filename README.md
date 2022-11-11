# MeowScript
A small interpreted easy extendable programming language.  
License: MIT

## Requirements
The requirements are:
- CMake and Make
- a  C++ compiler
- Either Windows or Linux (Other systems not tested, but might work to some extend)
- Only on Windows:  
-- Msys2  
-- MinGW

**Important: Windows build is currenlty officially NOT working!**

## Installing
Execute this commands in your shell:
```
$ git clone https://github.com/SirWolfi/MeowScript.git
$ cd MeowScript
$ mkdir build
$ cd build
$ cmake ..
$ make
```
If you use Windows, you can run the interpreter like this:
```
$ ./meow-script.exe --help
```
If you use Linux, use this:
```
$ ./meow-script --help
```

## How to use
Please read the wiki [here](https://github.com/SirWolfi/MeowScript/wiki) to learn more about MeowScript.

## Try it out
You can try out MeowScript live by using the `--shell` option.
