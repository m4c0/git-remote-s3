#ifndef GITWRAP_ODB_HPP
#define GITWRAP_ODB_HPP

#include "git-wrapper/odb_object.hpp"

#include "git2.h"

namespace git {
    class odb {
    public:
        odb(git_odb * o) : handler(o) {
        }
        ~odb() {
            if (handler) git_odb_free(handler);
        }

        odb(const odb &) = delete;
        odb(odb && o) : handler(nullptr) {
            std::swap(handler, o.handler);
        }

        odb & operator=(const odb &) = delete;
        odb & operator=(odb &&) = delete;

        odb_object read(const git_oid * oid) throw(error) {
            git_odb_object * obj;
            if (git_odb_read(&obj, handler, oid)) throw error();
            return odb_object(obj);
        }

    private:
        git_odb * handler;
    };
}

#endif

