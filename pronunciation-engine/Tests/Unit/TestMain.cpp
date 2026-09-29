#include <cstdio>

#include "TestFramework.h"

int main() {
    int passed = 0;
    int failed = 0;
    for (const auto& test : testfw::registry()) {
        try {
            test.run();
            ++passed;
            std::printf("[PASS] %s\n", test.name.c_str());
        } catch (const testfw::AssertionFailure& e) {
            ++failed;
            std::printf("[FAIL] %s: %s\n", test.name.c_str(), e.what());
        } catch (const std::exception& e) {
            ++failed;
            std::printf("[FAIL] %s: unexpected exception: %s\n", test.name.c_str(), e.what());
        }
    }
    std::printf("\n%d passed, %d failed, %d total\n", passed, failed, passed + failed);
    return failed == 0 ? 0 : 1;
}
