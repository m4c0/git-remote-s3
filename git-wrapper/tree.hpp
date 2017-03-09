#ifndef GITWRAP_TREE_HPP
#define GITWRAP_TREE_HPP

#include "git-wrapper/error.hpp"

namespace git {
    class tree {
    public:
        tree(git_repository * repo, const git_oid * oid) throw(error) : handler() {
            if (git_tree_lookup(&handler, repo, oid)) throw error();
        }
        ~tree() {
            git_tree_free(handler);
        }

        tree(const tree &) = delete;
        tree(tree && o) : handler(nullptr) {
            std::swap(handler, o.handler);
        }
        tree & operator=(const tree &) = delete;
        tree & operator==(tree &&) = delete;

        auto entrycount() {
            return git_tree_entrycount(handler);
        }
        auto entry_oid(size_t idx) {
            return git_tree_entry_id(git_tree_entry_byindex(handler, idx));
        }
        auto entry_type(size_t idx) {
            return git_tree_entry_type(git_tree_entry_byindex(handler, idx));
        }

    private:
        git_tree * handler;
    };
}

#endif

