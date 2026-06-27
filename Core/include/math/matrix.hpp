#pragma once
#include "math/vector.hpp"
#include <immintrin.h>
#include <numbers>

namespace zenyth::math {
	// Column-major right handed 4x4 matrix
	// Memory layout: col0(x,y,z,w), col1(x,y,z,w), col2(x,y,z,w), col3(x,y,z,w)
	class mat4 {
	public:
		mat4() noexcept;
		explicit mat4(float diagonal) noexcept;
		mat4(const vec4& col0, const vec4& col1, const vec4& col2, const vec4& col3) noexcept;

		static constexpr uint32_t width = 4;
		static constexpr uint32_t height = 4;
		static constexpr uint32_t size = width * height;

		[[nodiscard]] static mat4 identity() noexcept;
		[[nodiscard]] static mat4 zero() noexcept;

		[[nodiscard]] static mat4 from_basis(const vec3& right, const vec3& up, const vec3& forward) noexcept;
		[[nodiscard]] static mat4 from_axis_angle(const vec3& axis, float angle_rad) noexcept;
		[[nodiscard]] static mat4 from_euler(float pitch, float yaw, float roll) noexcept;
		[[nodiscard]] static mat4 from_scale(const vec3& scale) noexcept;
		[[nodiscard]] static mat4 from_translation(const vec3& t) noexcept;

		[[nodiscard]] mat4 operator+(const mat4& rhs) const noexcept;
		[[nodiscard]] mat4 operator-(const mat4& rhs) const noexcept;
		[[nodiscard]] mat4 operator*(float scalar) const noexcept;
		[[nodiscard]] mat4 operator/(float scalar) const noexcept;
		[[nodiscard]] mat4 operator*(const mat4& rhs) const noexcept;

		mat4& operator+=(const mat4& rhs) noexcept;
		mat4& operator-=(const mat4& rhs) noexcept;
		mat4& operator*=(float scalar) noexcept;
		mat4& operator/=(float scalar) noexcept;
		mat4& operator*=(const mat4& rhs) noexcept;

		[[nodiscard]] mat4  transpose() const noexcept;
		[[nodiscard]] float determinant() const noexcept;
		[[nodiscard]] mat4  inverse() const noexcept;
		[[nodiscard]] mat4  inverse_transpose() const noexcept;

		// Transform a position; applies translation
		[[nodiscard]] vec3 transform_point(const vec3& p)  const noexcept;
		// Transform a direction: ignores translation
		[[nodiscard]] vec3 transform_normal(const vec3& n) const noexcept;

		[[nodiscard]] vec4 operator*(const vec4& v) const noexcept;

		[[nodiscard]] static mat4 look_at(const vec3& eye, const vec3& center, const vec3& up) noexcept;
		[[nodiscard]] static mat4 perspective(float fov_y, float aspect, float near_z, float far_z) noexcept;
		[[nodiscard]] static mat4 orthographic(float left, float right, float bottom, float top, float near_z, float far_z) noexcept;
		[[nodiscard]] static mat4 perspective_reverse_z(float fov_y, float aspect, float near_z) noexcept;

		[[nodiscard]] float operator[](const std::size_t r, const std::size_t c) const noexcept { return m_data[c * 4 + r]; }
		[[nodiscard]] float& operator[](const std::size_t r, const std::size_t c) noexcept { return m_data[c * 4 + r];}

		[[nodiscard]] const void* data() const noexcept { return m_data; }
		[[nodiscard]] void* data() noexcept { return m_data; }

	private:
		mat4(__m128 c0, __m128 c1, __m128 c2, __m128 c3) noexcept;

		union {
			float m_data[16] {};
			__m128 m_col[4];
		};
	};
} // namespace zenyth::math
