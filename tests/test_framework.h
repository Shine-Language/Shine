#pragma once
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace testing {

struct TestCase { std::string name; std::function<void()> fn; };
inline std::vector<TestCase>& registry() { static std::vector<TestCase> t; return t; }
struct Registrar { Registrar(const std::string& n, std::function<void()> f) { registry().push_back({n, std::move(f)}); } };
struct Fail { std::string msg; };

inline int runAll() {
    int failed = 0;
    for (auto& t : registry()) {
        try { t.fn(); std::cout << "[PASS] " << t.name << "\n"; }
        catch (const Fail& f) { std::cout << "[FAIL] " << t.name << ": " << f.msg << "\n"; failed++; }
        catch (const std::exception& e) { std::cout << "[FAIL] " << t.name << ": " << e.what() << "\n"; failed++; }
    }
    std::cout << (registry().size() - failed) << "/" << registry().size() << " passed\n";
    return failed;
}

} // namespace testing

#define TEST(name) void name(); static testing::Registrar reg_##name(#name, name); void name()
#define ASSERT_TRUE(c) do { if (!(c)) throw testing::Fail{"ASSERT_TRUE failed: " #c}; } while (0)
#define ASSERT_EQ(a, b) do { if (!((a) == (b))) throw testing::Fail{#a " != " #b}; } while (0)
