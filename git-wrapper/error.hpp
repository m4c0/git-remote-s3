#ifndef GITWRAP_ERROR_HPP
#define GITWRAP_ERROR_HPP

#include "git2.h"

#include <exception>
#include <string>

namespace git {
    class error : public std::exception {
    public:
        error() {
            const git_error * err = giterr_last();
            message = err ? err->message : "unknown error";
        }

        const char * what() const throw() override {
            return message.c_str();
        }

    private:
        std::string message;
    };
}

#endif

