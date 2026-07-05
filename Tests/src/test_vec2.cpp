#include "TestFramework.hpp"
#include "math/vector.hpp"

using namespace zenyth::math;
using zenyth::test::tight_eps;
using zenyth::test::loose_eps;

// ---- vec2 tests ----

TEST_CASE("vec2: default constructor is zero") {
	vec2 v;
	CHECK_NEAR(v.x(), 0.0f, tight_eps);
	CHECK_NEAR(v.y(), 0.0f, tight_eps);
}

TEST_CASE("vec2: scalar constructor") {
	vec2 v(3.0f);
	CHECK_NEAR(v.x(), 3.0f, tight_eps);
	CHECK_NEAR(v.y(), 3.0f, tight_eps);
}

TEST_CASE("vec2: component constructor") {
	vec2 v(1.0f, 2.0f);
	CHECK_NEAR(v.x(), 1.0f, tight_eps);
	CHECK_NEAR(v.y(), 2.0f, tight_eps);
}

TEST_CASE("vec2: addition") {
	const vec2 a(1.0f, 2.0f);
	const vec2 b(3.0f, 4.0f);
	vec2 c = a + b;
	CHECK_NEAR(c.x(), 4.0f, tight_eps);
	CHECK_NEAR(c.y(), 6.0f, tight_eps);
}

TEST_CASE("vec2: subtraction") {
	const vec2 a(5.0f, 7.0f);
	const vec2 b(2.0f, 3.0f);
	vec2 c = a - b;
	CHECK_NEAR(c.x(), 3.0f, tight_eps);
	CHECK_NEAR(c.y(), 4.0f, tight_eps);
}

TEST_CASE("vec2: scalar multiply") {
	const vec2 v(2.0f, 3.0f);
	vec2 r = v * 4.0f;
	CHECK_NEAR(r.x(), 8.0f, tight_eps);
	CHECK_NEAR(r.y(), 12.0f, tight_eps);
}

TEST_CASE("vec2: scalar divide") {
	const vec2 v(8.0f, 6.0f);
	vec2 r = v / 2.0f;
	CHECK_NEAR(r.x(), 4.0f, tight_eps);
	CHECK_NEAR(r.y(), 3.0f, tight_eps);
}

TEST_CASE("vec2: compound add") {
	vec2 a(1.0f, 2.0f);
	a += vec2(3.0f, 4.0f);
	CHECK_NEAR(a.x(), 4.0f, tight_eps);
	CHECK_NEAR(a.y(), 6.0f, tight_eps);
}

TEST_CASE("vec2: negation") {
	const vec2 v(3.0f, -5.0f);
	vec2 n = -v;
	CHECK_NEAR(n.x(), -3.0f, tight_eps);
	CHECK_NEAR(n.y(), 5.0f, tight_eps);
}

TEST_CASE("vec2: dot product") {
	const vec2 a(1.0f, 2.0f);
	const vec2 b(3.0f, 4.0f);
	CHECK_NEAR(a.dot(b), 11.0f, tight_eps);  // 1*3 + 2*4 = 11
}

TEST_CASE("vec2: length") {
	const vec2 v(3.0f, 4.0f);
	CHECK_NEAR(v.length_sq(), 25.0f, tight_eps);
	CHECK_NEAR(v.length(), 5.0f, tight_eps);
}

TEST_CASE("vec2: mutable accessors") {
	vec2 v;
	v.x() = 7.0f;
	v.y() = 9.0f;
	CHECK_NEAR(v.x(), 7.0f, tight_eps);
	CHECK_NEAR(v.y(), 9.0f, tight_eps);
}
