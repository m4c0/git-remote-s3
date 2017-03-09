#include "git-wrapper/repository.hpp"

#include <aws/core/Aws.h>
#include <git2.h>

#include <iostream>
#include <regex>

static void _push(git::repository & repo, std::string line) {
    git::revwalk walk = repo.revwalk_new();

    do {
        if (line.length() == 0) break;

        std::regex push_regex { "push ([+])?([^:]+):(.+)" };
        std::smatch match;
        if (!std::regex_match(line, match, push_regex)) return;

        bool force = match[1].str() == "+";
        std::string local  = match[2].str();
        std::string remote = match[3].str();

        walk.push_ref(local);

        std::cout << "ok " << remote << std::endl;
    } while (std::getline(std::cin, line));

    git_oid * oid;
    while ((oid = walk.next()) != nullptr) {
        std::cerr << "oid: " << git_oid_tostr_s(oid) << std::endl;
    }

    std::cout << std::endl;
}

static void _run_command_loop() {
    git::repository repo;

    std::string line;
    while (std::getline(std::cin, line)) {
        //std::cerr << "got: " << line << std::endl;
        if (line == "") {
            return;
        } else if (line == "capabilities") {
            std::cout
                << "fetch"  << std::endl
                << "option" << std::endl
                << "push"   << std::endl
                << std::endl;
        } else if (line.find("list ") == 0) {
            std::cout << std::endl;
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

