# !! MOVED TO CODEBERG.ORG !!
**This is an old version, please visit the current version at [codeberg.org](https://codeberg.org/LabRicecat/meowscript)!**

# MeowScript

![Error Loading Image!](./assets/LogoLong.png)

<div id="badges">
   <img src="https://img.shields.io/github/v/release/LabRiceCat/MeowScript?label=latest&style=for-the-badge"/>
</div>
<img src="https://img.shields.io/github/license/LabRiceCat/MeowScript"/>  

A small interpreted, easy and extendable programming language.  

## Requirements
|             | Compiler            | Build tool     | Recommended |
|-------------|---------------------|----------------|-------------|
| Linux       | g++/clang++         | cmake          | ----------- |
| Windows*    | mingw64-g++/clang++ | nmake & cmake  | Msys2       |
| MacOS*      | ------------------- | -------------- | ----------- |

\*: still not fully supported

## Installing
Execute these commands in your shell to download and build MeowScript:
```sh
git clone https://github.com/LabRicecat/MeowScript.git
cd MeowScript
mkdir build
cd build
cmake ..
make
```
Now you can run the interpreter:
```sh
meow-script --help
```

## How to use
**[Read the wiki here](https://github.com/SirWolfi/MeowScript/wiki)**

## Try it out
You can try out MeowScript REPL by using the `--shell` option or make a `main.mws` and run it with
```sh
meow-script main.mws
```

## Credits
Cute cat on the PC by DALL-E 2.0
