#ifndef MEOWSCRIPT_IG_ERRORS_HPP
#define MEOWSCRIPT_IG_ERRORS_HPP

#include <exception>
#include <string>
#include "defs.hpp"

MEOWSCRIPT_HEADER_BEGIN

namespace errors {
    class MWSException : std::exception {
    public:
        unsigned int line = 0;
        virtual const char* what() const throw() {
            return "MWSException";
        }

        MWSException() {}
        MWSException(unsigned int line) : line(line) {}

        virtual const int get_line() const {
            return line;
        }
    };

    class MWSMessageException : public MWSException {
        std::string message = "";
    public:
        using MWSException::MWSException;
        virtual const char* what() const throw() {
            return message.c_str();
        }

        MWSMessageException() {}
        MWSMessageException(std::string msg, unsigned int line) {message = msg; MWSException::line = line;}
    };
}

MEOWSCRIPT_HEADER_END

#endif