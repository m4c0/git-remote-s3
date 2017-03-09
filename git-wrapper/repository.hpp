#ifndef GITWRAP_REPOSITORY_HPP
#define GITWRAP_REPOSITORY_HPP

#include "git-wrapper/commit.hpp"
#include "git-wrapper/error.hpp"
#include "git-wrapper/odb.hpp"
#include "git-wrapper/reference.hpp"
#include "git-wrapper/revwalk.hpp"
#include "git-wrapper/tree.hpp"

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

        commit commit_lookup(const git_oid * oid) {
            return commit(handler, oid);
        }
        reference reference_lookup(std::string spec) {
            return reference(handler, spec);
        }
        revwalk revwalk_new() {
            return revwalk(handler);
        }
        tree tree_lookup(const git_oid * oid) {
            return tree(handler, oid);
        }

        odb odb() throw(error) {
            git_odb * odb_ptr;
            if (git_repository_odb(&odb_ptr, handler)) throw error();
            return git::odb(odb_ptr);
        }

        git_repository * handler;
    };
}

#endif

