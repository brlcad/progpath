#include "progpath.h"
#include <iostream>
#include <string>
#include <vector>

int main() {
    char buf[1024];
    char* result = progpath(buf, sizeof(buf));

    if (!result) {
        std::cerr << "FAIL: progpath returned NULL" << std::endl;
        return 1;
    }

    std::string path(result);
    if (path.empty()) {
        std::cerr << "FAIL: progpath returned empty string" << std::endl;
        return 1;
    }

    std::cout << "PASS: progpath (C++) returned: " << path << std::endl;

    // Test with std::vector as buffer
    std::vector<char> vecBuf(1024);
    result = progpath(vecBuf.data(), vecBuf.size());
    if (!result) {
        std::cerr << "FAIL: progpath with vector buffer returned NULL" << std::endl;
        return 1;
    }
    
    std::cout << "PASS: progpath with vector buffer worked" << std::endl;

    return 0;
}
