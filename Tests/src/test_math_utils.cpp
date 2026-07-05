#include "TestFramework.hpp"
#include "math/utils.hpp"

using namespace zenyth::math;
using zenyth::test::tight_eps;
using zenyth::test::loose_eps;

TEST_CASE("math utils: rad/deg conversion") {
	CHECK_NEAR(rad(180.0f), PI, loose_eps);
	CHECK_NEAR(rad(90.0f), PI / 2.0f, loose_eps);
	CHECK_NEAR(rad(0.0f), 0.0f, tight_eps);
	CHECK_NEAR(deg(PI), 180.0f, loose_eps);
	CHECK_NEAR(deg(PI / 2.0f), 90.0f, loose_eps);
}

TEST_CASE("math utils: clamp") {
	CHECK_NEAR(clamp(5.0f, 0.0f, 10.0f), 5.0f, tight_eps);
	CHECK_NEAR(clamp(-1.0f, 0.0f, 10.0f), 0.0f, tight_eps);
	CHECK_NEAR(clamp(15.0f, 0.0f, 10.0f), 10.0f, tight_eps);
	CHECK_NEAR(clamp(0.0f, 0.0f, 10.0f), 0.0f, tight_eps);
	CHECK_NEAR(clamp(10.0f, 0.0f, 10.0f), 10.0f, tight_eps);
}
