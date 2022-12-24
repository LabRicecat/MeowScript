Hey, very cool that you want to contribute something!  
Before you do add stuff you might want to read this article though.

# How to contribute
Before you start to do anything, you might add an issue or start a discussion about what you plan to add and talk about it with the maintainers.  
This way you don't work hard for something that later on doesn't get added.  
Please be also aware that your work might need some changes before it can be pushed. This is not criticism but we (I) want keep the MeowScript standards.
Also Maybe consider making a **module** instead!

Important: Never ever commit a pull request to the `main` branch! Only to the `developer/` branch where all the ongoing work is stored, so we can asure nothing breaks when we release a new update. Thanks for understanding!

## When to contribute and when to make a module
Modules are for external things like making wrapers for libraries/frameworks or writing their own.  
There should be no need for a "better-meowscript" module, but for something like "curl-lib" or "algorihm-collection".

A commit is done when something essential that can't be done with modules gets added. This might include natice commands, new types or concepts.
To get permission to commit something is because of that also not that easy to get, we don't want to merge something broken that we have to fix afterwards.  

Modules are the users resposibility to function. But we have to assure that MeowScript keeps stable and save as well as portable.

# What a contribution should look like
First of all follow the instruction on the template.  
Please tell us exactly what you added, why you added it, how you added it (not that in detail but in general)  
and why it could not be done with a simple module.  

Be prepared to change stuff after a review, and keep in mind that we don't get paid for doing this, so we will take our time.  
It's also possible that we wont add your request at all, because of the reasons given above.

We are flattered that you want to add something and we try to best to listen to your suggestions!  

Now you are ready, if you are confident that a pull request is the perfect match, go for it.  
If you realize a module would be better, follow our template and add your ideas just as easy as watering your flowers!  
We are looking forward to your additions to this community!!  

## Styleguide
Please follow this styleguide so the codebase keeps readable, I know, I am not very consistant on this myself, but please try  
to keep function/class names as well as constants/macros and types compatible with this little guide.
### naming
- functions should be `snake_case`.
- macros should be `UPPER_SNAKE_CASE`.
- classes/structs are `PascalCase`, where it's prefered to give simple names (`Command`, `Function`, ...)
- variables and parameters should also be `snake_case`. 
- variables are if they are in a function or private class  also allowed to be shortly named, but keep it to an extend that make it maintainable.
- you use `struct` for things that are designed to store things and don't really act on their own. Usualy they are all `public`.
- you use `class` for things that are designed to act on their own and perform actions OR are exceptions (inhert from `std::exception`).
- files are all in lower case. Source file use `.cpp`, header files use `.hpp`.
- `if`/`while`/... have the expression as well as the curly brace inline (`if(...) {`)
- there is a space beween every expression, except with unary operators. (`x = x * !y`) (Only expection is when it has to be read as one thing like `work on b-4 with a*2 and ...`. In these occasions you might want to add braces around them though!)
- when using curly braces, but a space before them.
- when using normal brace expressions after a keyword/function, don't put a space before them.
- intendention is with 4 spaces.
- only one instruction per line except they must be read as one (RARE OCCURENCE)
- when calling a function, there is usually no space between the arguments, but this is not that harshly judged.

## Where to put what
### Desiging a new file
A header file should look like this:
```
#ifndef MEOWSCRIPT_IG_FILE_NAME_HPP
#define MEOWSCRIPT_IG_FILE_NAME_HPP

#include "defs.h"
// ... the headers you need

#include <iostream>
// not MeowScript headers ...

MEOWSCRIPT_HEADER_BEGIN

// your code goes here
int foo(int a, int b);
int bar();

MEOWSCRIPT_HEADER_END

#endif
```
These `filename.hpp` files should go into the `inc` folder.
The coresponding source file looks like this:
```
#include "../inc/filename.hpp"

MEOWSCRIPT_SOURCE_FILE

int MeowScript::foo(int a, int b) {
  // your code goes here
}

int MeowScript::bar() {
  // your code goes here
}
```
### File explanation
#### defs.hpp
Defines important macros and handles definitions for types/namespaces in general.
#### errors.hpp
Contains everything important for the MeowScript exceptions.  
Please don't add new ones, use the `MWSException` or `MWSMessageException`.
#### kittenlexer.hpp
External tool written by me. Different project, different style sometimes.  
This is frozen if nothing significent happens, so don't touch it if it's not neccesary.
#### meowscript.hpp
Includes all important headers.
#### reader.hpp
Handles the lexing the tokens as well as defining identification functions (`is_*`).
#### global.hpp
Defines global variables and some functions that modify these.
#### modules.hpp
Handles the module loading.
#### expressions.hpp
For the parsing process of expressions and defines what the operators do.
#### commands.hpp
Defines the behaviour of the different commands and stores them.
#### functions.hpp
Functions, parameters and events are defined here.
#### runner.hpp
The execution process of tokens happens with the functions defined here.
#### scopes.hpp
Handles the scoping and defines several functions to get serach down the scope stack.
#### tools.hpp
Utility functions or functions in general that don't need a new file on their own, yet are needed often.
#### variables.hpp
Defines how variables and typing works. Heavy file that defines many things needed.
#### list.hpp
Once in `variables.hpp` but moved to another file. Defines lists as well as dictionaries.  

The rule of thumb is always: look what does the thing you want to add fits best within, and then add it there.  

## When to use what
- **Function**  
A procedere that is needed somehwere else. Might or might not have a side effect. It it's not obvies, side effects should be documented.  
- **Class**  
A collection of functions and variables that is designed to do things on it's own. Inherintence, polimorphism, private, etc is allowed.
- **Struct**  
A collection of variables and functions that act on these members, that is designed to simply store data. Class features are mostly unwelcomed.
- **Namespace**  
Rarely, but sometimes needed when you have many things that would need a name prefix added if else (which is also by the way valid).  
- **File**  
Mostly when you have a bigger collection of new features, you create a new file to store them.
- **Macro**  
You can add a macro if you have a very specific value that might be changed later. If it's not globaly needed, you might also just declare a constant inside the function/make a static member, depending on what you need.  
Macros please go into the `defs.hpp` if not anything external like file specific functions are needed.

## Nodes
The bigger the things you add/modify/fix get, it gets harder to maintain, so please consider going to the discussion page first and talk about the changes you want to perform, so we can work out together a solution that works out!

Cya,
LabRicecat~
