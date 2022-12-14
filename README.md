# MeowScript

![Error Loading Image!](./assets/LogoLong.png)

<div id="badges">
   <img src="https://img.shields.io/github/v/release/LabRiceCat/MeowScript?label=latest&style=for-the-badge"/>
</div>
<img src="https://img.shields.io/github/license/LabRiceCat/MeowScript"/>  

A small interpreted, easy extendable programming language.  

## Requirements
|             | Compiler            | BuildTool     | Recomendet|
|-------------|---------------------|---------------|-----------|
| Linux       | g++/clang++         | cmake         |  -------  |
| Windows*    | mingw64-g++/clang++ | nmake & cmake |   Msys2   |
| MacOS*      | ---------           | ----------    |  -------  |

\* still not 100% supported

## Installing
Execute this commands in your shell to download and build MeowScript.
```
$ git clone https://github.com/LabRicecat/MeowScript.git
$ cd MeowScript
$ mkdir build
$ cd build
$ cmake ..
$ make
```
Now you can run the interpreter.
```
$ meow-script --help
```

## How to use
Read the wiki here.

**[To the Wiki Pages!](https://github.com/SirWolfi/MeowScript/wiki)**

## Try it out
You can try out MeowScript live by using the `--shell` option or make a `main.mws` and run it with
```
$ meow-script main.mws
```

## Credits
Cute cat on the PC by DALL-E 2.0
