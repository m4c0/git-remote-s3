#ifndef S3_WRAP_HPP
#define S3_WRAP_HPP

#include <fstream>
#include <sstream>

// Eventually, calls S3. For the sake of frugality, just mirrors at "build/"
class s3_wrap {
public:
    s3_wrap() {
    }

    std::string slurp(const std::string & key) {
        std::ifstream in { "build/" + key };
        std::stringstream buffer;
        buffer << in.rdbuf();
        return buffer.str();
    }

    void put(const std::string & key, std::string content) {
        std::ofstream out { "build/" + key };
        out << content;
    }
    void put(const std::string & key, const void * content, const size_t size) {
    }

private:
};

#endif

