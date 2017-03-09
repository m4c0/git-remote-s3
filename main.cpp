#include "git-wrapper/repository.hpp"
#include "ref_index.hpp"
#include "s3_wrap.hpp"

#include <aws/core/Aws.h>
#include <git2.h>

#include <iostream>
#include <regex>

static void _push_oid(git::repository & repo, const git_oid * oid) {
    auto odb = repo.odb();
    auto obj = odb.read(oid);
    s3_wrap().put(git_oid_tostr_s(oid), obj.data(), obj.size());

    switch (obj.type()) {
        case GIT_OBJ_COMMIT:
            _push_oid(repo, repo.commit_lookup(oid).tree_id());
            break;
        case GIT_OBJ_TREE: {
            auto tree = repo.tree_lookup(oid);
            for (int i = 0; i < tree.entrycount(); i++) {
                if (tree.entry_type(i) == GIT_OBJ_COMMIT) continue; // ignore submodules

                _push_oid(repo, tree.entry_oid(i));
            }
            break;
        }
        case GIT_OBJ_BLOB:
            // No further processing
            break;
        default:
            std::cerr << "unsupported: " << obj.type() << std::endl;
            break;
    }
}

static void _push(git::repository & repo, std::string line) {
    git::revwalk walk = repo.revwalk_new();
    ref_index idx;

    do {
        if (line.length() == 0) break;

        std::regex push_regex { "push ([+])?([^:]+):(.+)" };
        std::smatch match;
        if (!std::regex_match(line, match, push_regex)) return;

        bool force = match[1].str() == "+";
        std::string local  = match[2].str();
        std::string remote = match[3].str();

        walk.push_ref(local);

        auto target = repo.reference_lookup(local).target();
        idx[remote] = git_oid_tostr_s(target);
    } while (std::getline(std::cin, line));


    git_oid * oid;
    while ((oid = walk.next()) != nullptr) {
        _push_oid(repo, oid);
    }

    idx.publish();
}

static void _list() {
    s3_wrap s3;
    std::cout << s3.slurp("ref.txt") << std::endl;
}

static void _run_command_loop() {
    git::repository repo;

    std::string line;
    while (std::getline(std::cin, line)) {
        std::cerr << "got: " << line << std::endl;
        if (line == "") {
            return;
        } else if (line == "capabilities") {
            std::cout
                << "fetch"  << std::endl
                << "option" << std::endl
                << "push"   << std::endl
                << std::endl;
        } else if (line.find("list") == 0) {
            _list();
        } else if (line.find("option ") == 0) {
            std::cout << "unsupported" << std::endl;
        } else if (line.find("push ") == 0) {
            _push(repo, line);
        } else {
            std::cout << std::endl;
        }
    }
}

int main(int argc, char ** argv) {
    if (argc != 3) {
        std::cerr << "This tool is intended to be used thru GIT" << std::endl;
        return 1;
    }

    Aws::SDKOptions options;
    Aws::InitAPI(options);

    git_libgit2_init();

    _run_command_loop();

    git_libgit2_shutdown();
    
    Aws::ShutdownAPI(options);

    return 0;
}

