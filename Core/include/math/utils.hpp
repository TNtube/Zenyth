#pragma once
#include "constants.hpp"

namespace zenyth::math {
	[[nodiscard]] constexpr float rad(const float deg) noexcept { return deg * PI / 180.0f; }
	[[nodiscard]] constexpr float deg(const float rad) noexcept { return rad * 180.0f / PI; }
	[[nodiscard]] constexpr float clamp(const float val, const float min, const float max) noexcept { return std::min(max, std::max(val, min)); }
} // namespace zenyth::math
