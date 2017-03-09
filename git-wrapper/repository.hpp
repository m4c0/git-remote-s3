#ifndef GITWRAP_REPOSITORY_HPP
#define GITWRAP_REPOSITORY_HPP

#include "git-wrapper/error.hpp"
#include "git-wrapper/reference.hpp"
#include "git-wrapper/revwalk.hpp"

#include "git2.h"

#include <cstdlib>

namespace git {
    class repository {
    public:
        repository() throw(error) : handler(nullptr) {
            if (git_repository_open(&handler, std::getenv("GIT_DIR"))) throw error();
        }
        ~repository() {
            git_repository_free(handler);
        }

        repository(const repository &) = delete;
        repository(repository &&) = delete;
        repository & operator=(const repository &) = delete;
        repository & operator=(repository &&) = delete;

        reference reference_lookup(std::string spec) {
            return reference(handler, spec);
        }
        revwalk revwalk_new() {
            return revwalk(handler);
        }

        git_repository * handler;
    };
}

#endif

