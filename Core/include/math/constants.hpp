#pragma once
#include <numbers>

namespace zenyth::math {
	inline constexpr float epsilon = std::numeric_limits<float>::epsilon();
	inline constexpr float PI = std::numbers::pi;
	inline constexpr float PI2 = PI * 2.0f;
} // namespace zenyth::math
