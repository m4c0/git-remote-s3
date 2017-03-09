#ifndef GITWRAP_REVWALK_HPP
#define GITWRAP_REVWALK_HPP

#include "git-wrapper/error.hpp"

#include "git2.h"

namespace git {
    class revwalk {
    public:
        revwalk(git_repository * repo) throw(error) {
            if (git_revwalk_new(&handler, repo)) throw error();
        }
        ~revwalk() {
            if (handler) git_revwalk_free(handler);
        }

        revwalk(const revwalk & o) = delete;

        revwalk(revwalk && o) : handler(nullptr) {
            std::swap(o.handler, handler);
        }

        revwalk & operator=(const revwalk & o) = delete;
        revwalk & operator=(revwalk && o) = delete;

        void push_ref(const std::string & refspec) throw(error) {
            if (git_revwalk_push_ref(handler, refspec.c_str())) throw error();
        }

        git_oid * next() throw(error) {
            static git_oid oid;
            auto result = git_revwalk_next(&oid, handler);
            if (result == GIT_ITEROVER) return nullptr;
            if (result) throw error();
            return &oid;
        }

    private:
        git_revwalk * handler;
    };
}

#endif

