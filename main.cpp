#include <aws/core/Aws.h>
#include <git2.h>

#include <iostream>
#include <regex>

struct push_refspec {
    push_refspec(bool force, std::string local, std::string remote) : force(force), local_ref(local), remote_ref(remote) {
    }

    bool force;
    std::string local_ref;
    std::string remote_ref;
};

static bool _check_git_error(int error, std::string remote_ref) {
    if (!error) return false;

    const git_error * err = giterr_last();

    std::cout << "error " << remote_ref << " ";
    std::cout << (err ? err->message : "unknown error");
    std::cout << std::endl;

    return true;
}

#define GIT(x) _check_git_error(x, spec.remote_ref)
static void _push(git_repository * repo, const push_refspec & spec) {
	git_reference * ref;
    if (GIT(git_reference_lookup(&ref, repo, spec.local_ref.c_str()))) return;

    switch (git_reference_type(ref)) {
        case GIT_REF_OID: {
            char buf[100] = { 0 };
            git_oid_fmt(buf, git_reference_target(ref));
            std::cerr << buf << std::endl;
            break;
        }
        default:
            std::cout << "error " << spec.remote_ref << " unsupported" << std::endl;
            break;
    }

    git_reference_free(ref);

    std::cout << "ok " << spec.remote_ref << std::endl;
}

static void _run_command_loop() {
    const std::regex push_regex { "push ([+])?([^:]+):(.+)" };

    git_repository * repo = nullptr;
    if (git_repository_open(&repo, std::getenv("GIT_DIR"))) return;

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
            git_revwalk * walk;
            if (git_revwalk_new(&walk, repo)) return;

            do {
                if (line.length() == 0) break;

                std::smatch match;
                if (!std::regex_match(line, match, push_regex)) return;

                _push(repo, push_refspec(match[1].str() == "+", match[2].str(), match[3].str()));
            } while (std::getline(std::cin, line));

            git_revwalk_free(walk);
            std::cout << std::endl;
        } else {
            std::cout << std::endl;
        }
    }

    git_repository_free(repo);
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

