#ifndef REFINDEX_HPP
#define REFINDEX_HPP

#include "s3_wrap.hpp"

#include <iostream>
#include <sstream>
#include <unordered_map>

class ref_index {
public:
    ref_index() : ref_oid_map(), s3() {
    }

    std::string & operator[](std::string ref) {
        return ref_oid_map[ref];
    }

    void publish() {
        std::stringstream idx_file;
        std::stringstream result;
        for (auto & p : ref_oid_map) {
            idx_file << p.second << " " << p.first << std::endl;
            result << "ok " << p.first << std::endl;
        }
        s3.put("ref.txt", idx_file.str());
        std::cout << result.str() << std::endl << std::endl;
    }

private:
    s3_wrap s3;
    std::unordered_map<std::string, std::string> ref_oid_map;
};

#endif

