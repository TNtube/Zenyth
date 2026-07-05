#include "TestFramework.hpp"
#include "math/vector.hpp"

namespace zenyth::test {
	using namespace math;
	// ---- vec4 tests ----

	TEST_CASE("vec4: default constructor is zero") {
		const vec4 v;
		CHECK_NEAR(v.x(), 0.0f, tight_eps);
		CHECK_NEAR(v.y(), 0.0f, tight_eps);
		CHECK_NEAR(v.z(), 0.0f, tight_eps);
		CHECK_NEAR(v.w(), 0.0f, tight_eps);
	}

	TEST_CASE("vec4: scalar constructor") {
		const vec4 v(7.0f);
		CHECK_NEAR(v.x(), 7.0f, tight_eps);
		CHECK_NEAR(v.y(), 7.0f, tight_eps);
		CHECK_NEAR(v.z(), 7.0f, tight_eps);
		CHECK_NEAR(v.w(), 7.0f, tight_eps);
	}

	TEST_CASE("vec4: component constructor") {
		const vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
		CHECK_NEAR(v.x(), 1.0f, tight_eps);
		CHECK_NEAR(v.y(), 2.0f, tight_eps);
		CHECK_NEAR(v.z(), 3.0f, tight_eps);
		CHECK_NEAR(v.w(), 4.0f, tight_eps);
	}

	TEST_CASE("vec4: addition") {
		const vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
		const vec4 b(5.0f, 6.0f, 7.0f, 8.0f);
		const vec4 c = a + b;
		CHECK_NEAR(c.x(), 6.0f, tight_eps);
		CHECK_NEAR(c.y(), 8.0f, tight_eps);
		CHECK_NEAR(c.z(), 10.0f, tight_eps);
		CHECK_NEAR(c.w(), 12.0f, tight_eps);
	}

	TEST_CASE("vec4: subtraction") {
		const vec4 a(10.0f, 8.0f, 6.0f, 4.0f);
		const vec4 b(1.0f, 2.0f, 3.0f, 4.0f);
		const vec4 c = a - b;
		CHECK_NEAR(c.x(), 9.0f, tight_eps);
		CHECK_NEAR(c.y(), 6.0f, tight_eps);
		CHECK_NEAR(c.z(), 3.0f, tight_eps);
		CHECK_NEAR(c.w(), 0.0f, tight_eps);
	}

	TEST_CASE("vec4: scalar multiply") {
		const vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
		const vec4 r = v * 2.0f;
		CHECK_NEAR(r.x(), 2.0f, tight_eps);
		CHECK_NEAR(r.y(), 4.0f, tight_eps);
		CHECK_NEAR(r.z(), 6.0f, tight_eps);
		CHECK_NEAR(r.w(), 8.0f, tight_eps);
	}

	TEST_CASE("vec4: scalar divide") {
		const vec4 v(10.0f, 8.0f, 6.0f, 4.0f);
		const vec4 r = v / 2.0f;
		CHECK_NEAR(r.x(), 5.0f, tight_eps);
		CHECK_NEAR(r.y(), 4.0f, tight_eps);
		CHECK_NEAR(r.z(), 3.0f, tight_eps);
		CHECK_NEAR(r.w(), 2.0f, tight_eps);
	}

	TEST_CASE("vec4: negation") {
		const vec4 v(1.0f, -2.0f, 3.0f, -4.0f);
		const vec4 n = -v;
		CHECK_NEAR(n.x(), -1.0f, tight_eps);
		CHECK_NEAR(n.y(), 2.0f, tight_eps);
		CHECK_NEAR(n.z(), -3.0f, tight_eps);
		CHECK_NEAR(n.w(), 4.0f, tight_eps);
	}

	TEST_CASE("vec4: dot product") {
		const vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
		const vec4 b(5.0f, 6.0f, 7.0f, 8.0f);
		CHECK_NEAR(a.dot(b), 70.0f, loose_eps); // 1*5 + 2*6 + 3*7 + 4*8 = 5+12+21+32 = 70
	}

	TEST_CASE("vec4: length") {
		const vec4 v(1.0f, 2.0f, 2.0f, 0.0f);
		CHECK_NEAR(v.length_sq(), 9.0f, loose_eps);
		CHECK_NEAR(v.length(), 3.0f, loose_eps);
	}

	TEST_CASE("vec4: compound operators") {
		vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
		v += vec4(1.0f, 1.0f, 1.0f, 1.0f);
		CHECK_NEAR(v.x(), 2.0f, tight_eps);
		CHECK_NEAR(v.y(), 3.0f, tight_eps);
		CHECK_NEAR(v.z(), 4.0f, tight_eps);
		CHECK_NEAR(v.w(), 5.0f, tight_eps);

		v -= vec4(2.0f, 2.0f, 2.0f, 2.0f);
		CHECK_NEAR(v.x(), 0.0f, tight_eps);
		CHECK_NEAR(v.y(), 1.0f, tight_eps);
		CHECK_NEAR(v.z(), 2.0f, tight_eps);
		CHECK_NEAR(v.w(), 3.0f, tight_eps);

		v *= 3.0f;
		CHECK_NEAR(v.x(), 0.0f, tight_eps);
		CHECK_NEAR(v.y(), 3.0f, tight_eps);
		CHECK_NEAR(v.z(), 6.0f, tight_eps);
		CHECK_NEAR(v.w(), 9.0f, tight_eps);

		v /= 3.0f;
		CHECK_NEAR(v.x(), 0.0f, tight_eps);
		CHECK_NEAR(v.y(), 1.0f, tight_eps);
		CHECK_NEAR(v.z(), 2.0f, tight_eps);
		CHECK_NEAR(v.w(), 3.0f, tight_eps);
	}

	TEST_CASE("vec4: mutable accessors") {
		vec4 v;
		v.x() = 10.0f;
		v.y() = 20.0f;
		v.z() = 30.0f;
		v.w() = 40.0f;
		CHECK_NEAR(v.x(), 10.0f, tight_eps);
		CHECK_NEAR(v.y(), 20.0f, tight_eps);
		CHECK_NEAR(v.z(), 30.0f, tight_eps);
		CHECK_NEAR(v.w(), 40.0f, tight_eps);
	}

}
