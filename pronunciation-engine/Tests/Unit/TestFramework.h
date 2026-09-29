#pragma once

// Minimal, dependency-free test framework. The engine's Resources/ content
// pack and its build must work fully offline (docs/product-specification.md:
// "entire assessment and practice path works in airplane mode"), so the
// test suite itself avoids fetching a third-party test framework and stays
// self-contained.

#include <cmath>
#include <cstdio>
#include <exception>
#include <functional>
#include <string>
#include <vector>

namespace testfw {

struct TestCase {
    std::string name;
    std::function<void()> run;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Registrar {
    Registrar(const std::string& name, std::function<void()> run) { registry().push_back({name, std::move(run)}); }
};

struct AssertionFailure : std::exception {
    std::string message;
    explicit AssertionFailure(std::string m) : message(std::move(m)) {}
    const char* what() const noexcept override { return message.c_str(); }
};

inline int& failureCountForCurrentTest() {
    static int count = 0;
    return count;
}

} // namespace testfw

#define TEST(name)                                                                    \
    static void name();                                                               \
    static ::testfw::Registrar name##_registrar(#name, name);                         \
    static void name()

#define REQUIRE(cond)                                                                  \
    do {                                                                               \
        if (!(cond)) {                                                                 \
            throw ::testfw::AssertionFailure(std::string("REQUIRE failed: ") + #cond + \
                                              " at " __FILE__ ":" + std::to_string(__LINE__)); \
        }                                                                               \
    } while (0)

#define REQUIRE_NEAR(a, b, eps)                                                                     \
    do {                                                                                            \
        if (std::fabs((a) - (b)) > (eps)) {                                                         \
            throw ::testfw::AssertionFailure(std::string("REQUIRE_NEAR failed: ") + #a + " vs " + #b + \
                                              " at " __FILE__ ":" + std::to_string(__LINE__));      \
        }                                                                                            \
    } while (0)
