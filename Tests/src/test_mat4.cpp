#include "TestFramework.hpp"
#include "math/matrix.hpp"
#include "math/utils.hpp"


namespace zenyth::test {
#define MAT_AT(m, r, c) m[r, c]

	using namespace zenyth::math;

	TEST_CASE("mat4: default constructor is zeroed") {
		const mat4 m;
		for (int c = 0; c < 4; c++)
			for (int r = 0; r < 4; r++)
				CHECK_NEAR(MAT_AT(m, r, c), 0.0f, test::tight_eps);
	}

	TEST_CASE("mat4: diagonal constructor") {
		const mat4 m(1.0f);
		for (int c = 0; c < 4; c++)
			for (int r = 0; r < 4; r++) {
				const float expected = (r == c) ? 1.0f : 0.0f;
				CHECK_NEAR(MAT_AT(m, r, c), expected, tight_eps);
			}
	}

	TEST_CASE("mat4: identity") {
		const mat4 m = mat4::identity();
		for (int c = 0; c < 4; c++)
			for (int r = 0; r < 4; r++) {
				const float expected = (r == c) ? 1.0f : 0.0f;
				CHECK_NEAR(MAT_AT(m, r, c), expected, tight_eps);
			}
	}

	TEST_CASE("mat4: zero") {
		const mat4 m = mat4::zero();
		for (int c = 0; c < 4; c++)
			for (int r = 0; r < 4; r++)
				CHECK_NEAR(MAT_AT(m, r, c), 0.0f, tight_eps);
	}

	TEST_CASE("mat4: from_translation") {
		const mat4 m = mat4::from_translation(vec3(3.0f, 5.0f, 7.0f));

		// Should be identity with translation in column 3
		CHECK_NEAR(MAT_AT(m, 0, 0), 1.0f, tight_eps);
		CHECK_NEAR(MAT_AT(m, 1, 1), 1.0f, tight_eps);
		CHECK_NEAR(MAT_AT(m, 2, 2), 1.0f, tight_eps);
		CHECK_NEAR(MAT_AT(m, 3, 3), 1.0f, tight_eps);

		CHECK_NEAR(MAT_AT(m, 0, 3), 3.0f, tight_eps);  // tx
		CHECK_NEAR(MAT_AT(m, 1, 3), 5.0f, tight_eps);  // ty
		CHECK_NEAR(MAT_AT(m, 2, 3), 7.0f, tight_eps);  // tz

		// Off-diagonal should be zero
		CHECK_NEAR(MAT_AT(m, 0, 1), 0.0f, tight_eps);
		CHECK_NEAR(MAT_AT(m, 0, 2), 0.0f, tight_eps);
		CHECK_NEAR(MAT_AT(m, 1, 0), 0.0f, tight_eps);
		CHECK_NEAR(MAT_AT(m, 3, 0), 0.0f, tight_eps);
	}

	TEST_CASE("mat4: identity * identity = identity") {
		const mat4 I = mat4::identity();
		const mat4 r = I * I;
		for (int c = 0; c < 4; c++)
			for (int r_ = 0; r_ < 4; r_++) {
				const float expected = (r_ == c) ? 1.0f : 0.0f;
				CHECK_NEAR(MAT_AT(r, r_, c), expected, tight_eps);
			}
	}

	TEST_CASE("mat4: multiply identity preserves matrix") {
		const mat4 I = mat4::identity();
		const mat4 t = mat4::from_translation(vec3(1.0f, 2.0f, 3.0f));
		const mat4 r = I * t;
		CHECK_NEAR(MAT_AT(r, 0, 3), 1.0f, tight_eps);
		CHECK_NEAR(MAT_AT(r, 1, 3), 2.0f, tight_eps);
		CHECK_NEAR(MAT_AT(r, 2, 3), 3.0f, tight_eps);
	}

	TEST_CASE("mat4: translation composition") {
		// Two translations should add
		const mat4 t1 = mat4::from_translation(vec3(1.0f, 0.0f, 0.0f));
		const mat4 t2 = mat4::from_translation(vec3(0.0f, 2.0f, 3.0f));
		const mat4 r = t1 * t2;
		CHECK_NEAR(MAT_AT(r, 0, 3), 1.0f, tight_eps);
		CHECK_NEAR(MAT_AT(r, 1, 3), 2.0f, tight_eps);
		CHECK_NEAR(MAT_AT(r, 2, 3), 3.0f, tight_eps);
	}

	TEST_CASE("mat4: matrix-vector multiply with identity") {
		const mat4 I = mat4::identity();
		const vec4 v(1.0f, 2.0f, 3.0f, 1.0f);
		vec4 r = I * v;
		CHECK_NEAR(r.x(), 1.0f, tight_eps);
		CHECK_NEAR(r.y(), 2.0f, tight_eps);
		CHECK_NEAR(r.z(), 3.0f, tight_eps);
		CHECK_NEAR(r.w(), 1.0f, tight_eps);
	}

	TEST_CASE("mat4: translation transforms point") {
		const mat4 t = mat4::from_translation(vec3(10.0f, 20.0f, 30.0f));
		const vec4 p(1.0f, 2.0f, 3.0f, 1.0f);  // w=1 for point
		const vec4 r = t * p;
		CHECK_NEAR(r.x(), 11.0f, loose_eps);
		CHECK_NEAR(r.y(), 22.0f, loose_eps);
		CHECK_NEAR(r.z(), 33.0f, loose_eps);
		CHECK_NEAR(r.w(), 1.0f, loose_eps);
	}

	TEST_CASE("mat4: translation does not affect direction") {
		const mat4 t = mat4::from_translation(vec3(10.0f, 20.0f, 30.0f));
		const vec4 d(1.0f, 0.0f, 0.0f, 0.0f);  // w=0 for direction
		vec4 r = t * d;
		CHECK_NEAR(r.x(), 1.0f, loose_eps);
		CHECK_NEAR(r.y(), 0.0f, loose_eps);
		CHECK_NEAR(r.z(), 0.0f, loose_eps);
		CHECK_NEAR(r.w(), 0.0f, loose_eps);
	}

	TEST_CASE("mat4: perspective produces valid projection") {
		constexpr float fov = rad(90.0f);
		constexpr float aspect = 16.0f / 9.0f;
		mat4 p = mat4::perspective(fov, aspect, 0.1f, 100.0f);

		// Diagonal should be non-zero
		CHECK_FALSE(MAT_AT(p, 0, 0) == 0.0f);
		CHECK_FALSE(MAT_AT(p, 1, 1) == 0.0f);
		CHECK_FALSE(MAT_AT(p, 2, 2) == 0.0f);

		// m[3,2] should be -1 for right-handed
		CHECK_NEAR(MAT_AT(p, 3, 2), -1.0f, tight_eps);

		// m[3,3] should be 0 for perspective (not ortho)
		CHECK_NEAR(MAT_AT(p, 3, 3), 0.0f, tight_eps);
	}

	TEST_CASE("mat4: look_at produces valid view matrix") {
		vec3 eye(0.0f, 0.0f, 5.0f);
		vec3 center(0.0f, 0.0f, 0.0f);
		vec3 up(0.0f, 1.0f, 0.0f);

		mat4 v = mat4::look_at(eye, center, up);

		// Transform the eye point: should map to origin
		vec4 eye_h(0.0f, 0.0f, 5.0f, 1.0f);
		vec4 r = v * eye_h;
		CHECK_NEAR(r.x(), 0.0f, loose_eps);
		CHECK_NEAR(r.y(), 0.0f, loose_eps);
		CHECK_NEAR(r.z(), 0.0f, loose_eps);

		// m[3,3] should be 1 (affine transform)
		CHECK_NEAR(MAT_AT(v, 3, 3), 1.0f, tight_eps);
	}
}