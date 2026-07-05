#include "TestFramework.hpp"
#include "math/vector.hpp"

using namespace zenyth::math;
using zenyth::test::tight_eps;
using zenyth::test::loose_eps;

// ---- vec3 tests ----

TEST_CASE("vec3: default constructor is zero") {
	const vec3 v;
	CHECK_NEAR(v.x(), 0.0f, tight_eps);
	CHECK_NEAR(v.y(), 0.0f, tight_eps);
	CHECK_NEAR(v.z(), 0.0f, tight_eps);
}

TEST_CASE("vec3: scalar constructor") {
	const vec3 v(5.0f);
	CHECK_NEAR(v.x(), 5.0f, tight_eps);
	CHECK_NEAR(v.y(), 5.0f, tight_eps);
	CHECK_NEAR(v.z(), 5.0f, tight_eps);
}

TEST_CASE("vec3: component constructor") {
	const vec3 v(1.0f, 2.0f, 3.0f);
	CHECK_NEAR(v.x(), 1.0f, tight_eps);
	CHECK_NEAR(v.y(), 2.0f, tight_eps);
	CHECK_NEAR(v.z(), 3.0f, tight_eps);
}

TEST_CASE("vec3: addition") {
	const vec3 a(1.0f, 2.0f, 3.0f);
	const vec3 b(4.0f, 5.0f, 6.0f);
	const vec3 c = a + b;
	CHECK_NEAR(c.x(), 5.0f, tight_eps);
	CHECK_NEAR(c.y(), 7.0f, tight_eps);
	CHECK_NEAR(c.z(), 9.0f, tight_eps);
}

TEST_CASE("vec3: subtraction") {
	const vec3 a(10.0f, 8.0f, 6.0f);
	const vec3 b(1.0f, 2.0f, 3.0f);
	const vec3 c = a - b;
	CHECK_NEAR(c.x(), 9.0f, tight_eps);
	CHECK_NEAR(c.y(), 6.0f, tight_eps);
	CHECK_NEAR(c.z(), 3.0f, tight_eps);
}

TEST_CASE("vec3: scalar multiply") {
	const vec3 v(2.0f, 3.0f, 4.0f);
	const vec3 r = v * 3.0f;
	CHECK_NEAR(r.x(), 6.0f, tight_eps);
	CHECK_NEAR(r.y(), 9.0f, tight_eps);
	CHECK_NEAR(r.z(), 12.0f, tight_eps);
}

TEST_CASE("vec3: scalar divide") {
	const vec3 v(9.0f, 6.0f, 3.0f);
	const vec3 r = v / 3.0f;
	CHECK_NEAR(r.x(), 3.0f, tight_eps);
	CHECK_NEAR(r.y(), 2.0f, tight_eps);
	CHECK_NEAR(r.z(), 1.0f, tight_eps);
}

TEST_CASE("vec3: negation") {
	const vec3 v(1.0f, -2.0f, 3.0f);
	const vec3 n = -v;
	CHECK_NEAR(n.x(), -1.0f, tight_eps);
	CHECK_NEAR(n.y(), 2.0f, tight_eps);
	CHECK_NEAR(n.z(), -3.0f, tight_eps);
}

TEST_CASE("vec3: dot product") {
	const vec3 a(1.0f, 2.0f, 3.0f);
	const vec3 b(4.0f, 5.0f, 6.0f);
	CHECK_NEAR(a.dot(b), 32.0f, loose_eps); // 1*4 + 2*5 + 3*6 = 32
}

TEST_CASE("vec3: dot product orthogonal vectors") {
	const vec3 a(1.0f, 0.0f, 0.0f);
	const vec3 b(0.0f, 1.0f, 0.0f);
	CHECK_NEAR(a.dot(b), 0.0f, tight_eps);
}

TEST_CASE("vec3: cross product basis vectors") {
	const vec3 x(1.0f, 0.0f, 0.0f);
	const vec3 y(0.0f, 1.0f, 0.0f);
	const vec3 z = x.cross(y);
	// x cross y = z
	CHECK_NEAR(z.x(), 0.0f, tight_eps);
	CHECK_NEAR(z.y(), 0.0f, tight_eps);
	CHECK_NEAR(z.z(), 1.0f, tight_eps);
}

TEST_CASE("vec3: cross product anti-commutativity") {
	const vec3 a(1.0f, 2.0f, 3.0f);
	const vec3 b(4.0f, 5.0f, 6.0f);
	const vec3 ab = a.cross(b);
	const vec3 ba = b.cross(a);
	CHECK_NEAR(ab.x(), -ba.x(), loose_eps);
	CHECK_NEAR(ab.y(), -ba.y(), loose_eps);
	CHECK_NEAR(ab.z(), -ba.z(), loose_eps);
}

TEST_CASE("vec3: cross product known result") {
	const vec3 a(2.0f, 3.0f, 4.0f);
	const vec3 b(5.0f, 6.0f, 7.0f);
	const vec3 c = a.cross(b);
	// (3*7 - 4*6, 4*5 - 2*7, 2*6 - 3*5) = (-3, 6, -3)
	CHECK_NEAR(c.x(), -3.0f, loose_eps);
	CHECK_NEAR(c.y(),  6.0f, loose_eps);
	CHECK_NEAR(c.z(), -3.0f, loose_eps);
}

TEST_CASE("vec3: length") {
	const vec3 v(2.0f, 3.0f, 6.0f);
	CHECK_NEAR(v.length_sq(), 49.0f, loose_eps);
	CHECK_NEAR(v.length(), 7.0f, loose_eps);
}

TEST_CASE("vec3: mutable accessors") {
	vec3 v;
	v.x() = 1.0f;
	v.y() = 2.0f;
	v.z() = 3.0f;
	CHECK_NEAR(v.x(), 1.0f, tight_eps);
	CHECK_NEAR(v.y(), 2.0f, tight_eps);
	CHECK_NEAR(v.z(), 3.0f, tight_eps);
}
