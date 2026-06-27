#include "pch.hpp"
#include "math/vector.hpp"

#include <immintrin.h>

namespace zenyth::math {
#pragma region vec2
	vec2::vec2(const float x, const float y)
		: m_x(x), m_y(y) {}

	vec2::vec2(const float scalar)
		: m_x(scalar), m_y(scalar) {}

	vec2::vec2()
		: m_x(0), m_y(0) {}

	float vec2::x() const noexcept { return m_x; }
	float vec2::y() const noexcept { return m_y; }

	float& vec2::x() noexcept { return m_x; }
	float& vec2::y() noexcept { return m_y; }

	vec2 vec2::operator+(const vec2& other) const noexcept { return { m_x + other.m_x, m_y + other.m_y }; }
	vec2 vec2::operator-(const vec2& other) const noexcept { return { m_x - other.m_x, m_y - other.m_y }; }
	vec2 vec2::operator*(const float scalar) const noexcept { return { m_x * scalar, m_y * scalar }; }
	vec2 vec2::operator/(const float scalar) const noexcept { return { m_x / scalar, m_y / scalar }; }

	void vec2::operator+=(const vec2& other) noexcept { m_x += other.m_x; m_y += other.m_y; }
	void vec2::operator-=(const vec2& other) noexcept { m_x -= other.m_x; m_y -= other.m_y; }
	void vec2::operator*=(const float scalar) noexcept { m_x *= scalar; m_y *= scalar; }
	void vec2::operator/=(const float scalar) noexcept { m_x /= scalar; m_y /= scalar; }

	vec2 vec2::operator-() const noexcept { return { -m_x, -m_y }; }

	float vec2::dot(const vec2& other) const noexcept { return m_x * other.m_x + m_y * other.m_y; }
	float vec2::length_sq() const noexcept { return dot(*this); }
	float vec2::length() const noexcept { return std::sqrtf(length_sq()); }
#pragma endregion

#pragma region vec3
	vec3::vec3(const float x, const float y, const float z)
		: m_simd(_mm_set_ps(0.0f, z, y, x)) {
	}

	vec3::vec3(const float scalar)
		: m_simd(_mm_set_ps(0.0f, scalar, scalar, scalar)) {
	}

	vec3::vec3()
		: m_simd(_mm_setzero_ps()) {
	}

	float vec3::x() const noexcept { return m_components.m_x; }
	float vec3::y() const noexcept { return m_components.m_z; }
	float vec3::z() const noexcept { return m_components.m_z; }

	float& vec3::x() noexcept { return m_components.m_x; }
	float& vec3::y() noexcept { return m_components.m_y; }
	float& vec3::z() noexcept { return m_components.m_z; }

	vec3 vec3::operator+(const vec3& other) const noexcept { return vec3{_mm_add_ps(m_simd, other.m_simd)}; }
	vec3 vec3::operator-(const vec3& other) const noexcept { return vec3{_mm_sub_ps(m_simd, other.m_simd)}; }
	vec3 vec3::operator*(const float scalar) const noexcept { return vec3{_mm_mul_ps(m_simd, _mm_set1_ps(scalar))}; }
	vec3 vec3::operator/(const float scalar) const noexcept { return vec3{_mm_div_ps(m_simd, _mm_set1_ps(scalar))}; }

	void vec3::operator+=(const vec3& other) noexcept { m_simd = _mm_add_ps(m_simd, other.m_simd); }
	void vec3::operator-=(const vec3& other) noexcept { m_simd = _mm_sub_ps(m_simd, other.m_simd); }
	void vec3::operator*=(const float scalar) noexcept { m_simd = _mm_mul_ps(m_simd, _mm_set1_ps(scalar)); }
	void vec3::operator/=(const float scalar) noexcept { m_simd = _mm_div_ps(m_simd, _mm_set1_ps(scalar)); }

	vec3 vec3::operator-() const noexcept { return vec3{_mm_sub_ps(_mm_setzero_ps(), m_simd)}; }

	vec3 vec3::cross(const vec3& other) const noexcept
	{
		// left term
		const __m128 a_yzx = _mm_shuffle_ps(m_simd, m_simd, _MM_SHUFFLE(3, 0, 2, 1));
		const __m128 b_zxy = _mm_shuffle_ps(other.m_simd, other.m_simd, _MM_SHUFFLE(3, 1, 0, 2));
		const __m128 left = _mm_mul_ps(a_yzx, b_zxy);

		// right term
		const __m128 a_zxy = _mm_shuffle_ps(m_simd, m_simd, _MM_SHUFFLE(3, 1, 0, 2));
		const __m128 b_yzx = _mm_shuffle_ps(other.m_simd, other.m_simd, _MM_SHUFFLE(3, 0, 2, 1));
		const __m128 right = _mm_mul_ps(a_zxy, b_yzx);

		return vec3{_mm_sub_ps(left, right)};
	}

	float vec3::dot(const vec3& other) const noexcept { return _mm_cvtss_f32(_mm_dp_ps(m_simd, other.m_simd, 0xF1)); }
	float vec3::length_sq() const noexcept { return dot(*this); }
	float vec3::length() const noexcept { return std::sqrtf(length_sq()); }
#pragma endregion

#pragma region vec4
	vec4::vec4(const float x, const float y, const float z, const float w)
		: m_simd(_mm_set_ps(w, z, y, x)) {}

	vec4::vec4(const float scalar)
		: m_simd(_mm_set_ps1(scalar)) {}

	vec4::vec4()
		: m_simd(_mm_setzero_ps()) {}

	float vec4::x() const noexcept { return m_components.m_x; }
	float vec4::y() const noexcept { return m_components.m_y; }
	float vec4::z() const noexcept { return m_components.m_z; }
	float vec4::w() const noexcept { return m_components.m_w; }

	float& vec4::x() noexcept { return m_components.m_x; }
	float& vec4::y() noexcept { return m_components.m_y; }
	float& vec4::z() noexcept { return m_components.m_z; }
	float& vec4::w() noexcept { return m_components.m_w; }

	vec4 vec4::operator+(const vec4& other) const noexcept { return vec4{_mm_add_ps(m_simd, other.m_simd)}; }
	vec4 vec4::operator-(const vec4& other) const noexcept { return vec4{_mm_sub_ps(m_simd, other.m_simd)}; }
	vec4 vec4::operator*(const float scalar) const noexcept { return vec4{_mm_mul_ps(m_simd, _mm_set1_ps(scalar))}; }
	vec4 vec4::operator/(const float scalar) const noexcept { return vec4{_mm_div_ps(m_simd, _mm_set1_ps(scalar))}; }

	void vec4::operator+=(const vec4& other) noexcept { m_simd = _mm_add_ps(m_simd, other.m_simd); }
	void vec4::operator-=(const vec4& other) noexcept { m_simd = _mm_sub_ps(m_simd, other.m_simd); }
	void vec4::operator*=(const float scalar) noexcept { m_simd = _mm_mul_ps(m_simd, _mm_set1_ps(scalar)); }
	void vec4::operator/=(const float scalar) noexcept { m_simd = _mm_div_ps(m_simd, _mm_set1_ps(scalar)); }

	vec4 vec4::operator-() const noexcept { return vec4{_mm_sub_ps(_mm_setzero_ps(), m_simd)}; }

	float vec4::dot(const vec4& other) const noexcept { return _mm_cvtss_f32(_mm_dp_ps(m_simd, other.m_simd, 0xF1)); }
	float vec4::length_sq() const noexcept { return dot(*this); }
	float vec4::length() const noexcept { return std::sqrtf(length_sq()); }
#pragma endregion
}