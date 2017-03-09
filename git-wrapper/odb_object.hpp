#ifndef GITWRAP_ODBOBJECT_HPP
#define GITWRAP_ODBOBJECT_HPP

#include "git2.h"

namespace git {
    class odb_object {
    public:
        odb_object(git_odb_object * o) : handler(o) {
        }
        ~odb_object() {
            if (handler) git_odb_object_free(handler);
        }

        odb_object(const odb_object &) = delete;
        odb_object(odb_object && o) : handler(nullptr) {
            std::swap(handler, o.handler);
        }

        odb_object & operator=(const odb_object &) = delete;
        odb_object & operator=(odb_object &&) = delete;

        auto data() {
            return git_odb_object_data(handler);
        }
        auto size() {
            return git_odb_object_size(handler);
        }
        auto type() {
            return git_odb_object_type(handler);
        }

    private:
        git_odb_object * handler;
    };
}

#endif

