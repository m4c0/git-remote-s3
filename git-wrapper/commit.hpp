#ifndef GITWRAP_COMMIT_HPP
#define GITWRAP_COMMIT_HPP

#include "git-wrapper/error.hpp"

namespace git {
    class commit {
    public:
        commit(git_repository * repo, const git_oid * oid) throw(error) : handler() {
            if (git_commit_lookup(&handler, repo, oid)) throw error();
        }
        ~commit() {
            git_commit_free(handler);
        }

        commit(const commit &) = delete;
        commit(commit && o) : handler(nullptr) {
            std::swap(handler, o.handler);
        }
        commit & operator=(const commit &) = delete;
        commit & operator==(commit &&) = delete;

        auto tree_id() {
            return git_commit_tree_id(handler);
        }

    private:
        git_commit * handler;
    };
}

#endif

