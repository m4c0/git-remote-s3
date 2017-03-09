#ifndef GITWRAP_REFERENCE_HPP
#define GITWRAP_REFERENCE_HPP

#include "git2.h"

namespace git {
    class reference {
    public:
        reference(git_repository * repo, std::string spec) throw(error) {
            if (git_reference_lookup(&handler, repo, spec.c_str())) throw error();
        }
        ~reference() {
            if (handler) git_reference_free(handler);
        }

        reference(const reference &) = delete;

        reference(reference && o) : handler(nullptr) {
            std::swap(handler, o.handler);
        }

        reference & operator=(const reference &) = delete;
        reference & operator=(reference &&) = delete;

        auto target() {
            return git_reference_target(handler);
        }
        auto type() {
            return git_reference_type(handler);
        }

    private:
        git_reference * handler;
    };
}

#endif

