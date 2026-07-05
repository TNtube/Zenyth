#pragma once
// Minimal test framework
// Zero dependencies, header-only, auto-registering test cases.
//
// Usage:
//   TEST_CASE("my test") {
//       CHECK(1 + 1 == 2);
//       CHECK_NEAR(3.14f, 3.14f, 0.001f);
//       CHECK_FALSE(0 == 1);
//   }
//
// All TEST_CASEs auto-register. main() calls zenyth::test::run_all().

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace zenyth::test {

struct TestCase {
	std::string name;
	std::function<void()> func;
};

struct TestState {
	int checks_passed  = 0;
	int checks_failed  = 0;
	const char* current_test = nullptr;
};

inline std::vector<TestCase>& registry() {
	static std::vector<TestCase> cases;
	return cases;
}

inline TestState& state() {
	static TestState s;
	return s;
}

struct AutoRegister {
	AutoRegister(const char* name, std::function<void()> fn) {
		registry().push_back({name, std::move(fn)});
	}
};

inline void report_failure(const char* expr, const char* file, int line) {
	std::fprintf(stderr, "  FAILED: %s\n    at %s:%d\n", expr, file, line);
	state().checks_failed++;
}

inline void report_near_failure(const char* a_expr, const char* b_expr, const float a, const float b, const float eps, const char* file, const int line) {
	std::fprintf(stderr, "  FAILED: %s ~= %s  (%.9g vs %.9g, eps=%.9g)\n    at %s:%d\n", a_expr, b_expr, a, b, eps, file, line);
	state().checks_failed++;
}

inline int run_all() {
	auto& cases = registry();
	auto& [checks_passed, checks_failed, current_test] = state();
	int tests_failed = 0;

	std::printf("Running %zu test(s)...\n\n", cases.size());

	for (auto& [name, func] : cases) {
		current_test = name.c_str();
		const int before = checks_failed;
		std::printf("[RUN ] %s\n", name.c_str());
		func();
		if (checks_failed > before) {
			std::printf("[FAIL] %s\n\n", name.c_str());
			tests_failed++;
		} else {
			std::printf("[ OK ] %s\n\n", name.c_str());
		}
	}

	std::printf("---------------------------\n");
	std::printf("Results: %d passed, %d failed (%d checks total)\n",
		checks_passed, checks_failed,
		checks_passed + checks_failed);

	if (tests_failed > 0) {
		std::printf("%d test(s) FAILED.\n", tests_failed);
		return 1;
	}
	std::printf("All tests passed.\n");
	return 0;
}

// Test tolerances for floating-point comparisons
inline constexpr float tight_eps = 1e-6f;  // single-op: accessors, add, subtract
inline constexpr float loose_eps = 1e-5f;  // multi-op: dot, cross, length, matrix transforms

} // namespace zenyth::test

#define ZTEST_CONCAT_(a, b) a##b
#define ZTEST_CONCAT(a, b) ZTEST_CONCAT_(a, b)

#define TEST_CASE(name)                                                         \
	static void ZTEST_CONCAT(ztest_func_, __LINE__)();                          \
	static ::zenyth::test::AutoRegister                                         \
		ZTEST_CONCAT(ztest_reg_, __LINE__)(                                     \
			name, ZTEST_CONCAT(ztest_func_, __LINE__));                         \
	static void ZTEST_CONCAT(ztest_func_, __LINE__)()

#define CHECK(expr)                                                             \
	do {                                                                        \
		if (expr) { ::zenyth::test::state().checks_passed++; }                  \
		else { ::zenyth::test::report_failure(#expr, __FILE__, __LINE__); }     \
	} while (0)

#define CHECK_FALSE(expr)                                                       \
	do {                                                                        \
		if (!(expr)) { ::zenyth::test::state().checks_passed++; }               \
		else { ::zenyth::test::report_failure("!" #expr, __FILE__, __LINE__); } \
	} while (0)

#define CHECK_NEAR(a, b, eps)                                                   \
	do {                                                                        \
		float ztest_a_ = (a); float ztest_b_ = (b); float ztest_e_ = (eps);     \
		if (std::fabs(ztest_a_ - ztest_b_) <= ztest_e_) {                       \
			::zenyth::test::state().checks_passed++;                            \
		} else {                                                                \
			::zenyth::test::report_near_failure(                                \
				#a, #b, ztest_a_, ztest_b_, ztest_e_, __FILE__, __LINE__);      \
		}                                                                       \
	} while (0)
