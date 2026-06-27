#include "pch.hpp"
#include "math/matrix.hpp"

namespace zenyth::math {
	mat4::mat4() noexcept {}
	mat4::mat4(float diagonal) noexcept {}

	mat4::mat4(const vec4 &col0, const vec4 &col1, const vec4 &col2, const vec4 &col3) noexcept
		: m_col {col0.m_simd, col1.m_simd, col2.m_simd, col3.m_simd} {}

	mat4 mat4::identity() noexcept {
		return {};
	}

	mat4 mat4::zero() noexcept {
		return {};
	}

	mat4 mat4::from_basis(const vec3 &right, const vec3 &up, const vec3 &forward) noexcept {
		return {};
	}

	mat4 mat4::from_axis_angle(const vec3 &axis, float angle_rad) noexcept {
		return {};
	}

	mat4 mat4::from_euler(float pitch, float yaw, float roll) noexcept {
		return {};
	}

	mat4 mat4::from_scale(const vec3 &scale) noexcept {
		return {};
	}

	mat4 mat4::from_translation(const vec3 &t) noexcept {
		return {};
	}

	mat4 mat4::operator+(const mat4 &rhs) const noexcept {
		return {};
	}

	mat4 mat4::operator-(const mat4 &rhs) const noexcept {
		return {};
	}

	mat4 mat4::operator*(float scalar) const noexcept {
		return {};
	}

	mat4 mat4::operator/(float scalar) const noexcept {
		return {};
	}

	mat4 mat4::operator*(const mat4 &rhs) const noexcept {
		return {};
	}

	mat4& mat4::operator+=(const mat4 &rhs) noexcept {
		return *this;
	}

	mat4& mat4::operator-=(const mat4 &rhs) noexcept {
		return *this;
	}

	mat4& mat4::operator*=(float scalar) noexcept {
		return *this;
	}

	mat4& mat4::operator/=(float scalar) noexcept {
		return *this;
	}

	mat4& mat4::operator*=(const mat4 &rhs) noexcept {
		return *this;
	}

	mat4 mat4::transpose() const noexcept {
		return *this;
	}

	float mat4::determinant() const noexcept {
		return m_data[0];
	}

	mat4 mat4::inverse() const noexcept {
		return *this;
	}

	mat4 mat4::inverse_transpose() const noexcept {
		return *this;
	}

	vec3 mat4::transform_point(const vec3 &p) const noexcept {
		return {m_data[0], m_data[1], m_data[2]};
	}

	vec3 mat4::transform_normal(const vec3 &n) const noexcept {
		return {m_data[0], m_data[1], m_data[2]};
	}

	vec4 mat4::operator*(const vec4 &v) const noexcept {
		return {m_data[0], m_data[1], m_data[2], m_data[4]};
	}

	mat4 mat4::look_at(const vec3 &eye, const vec3 &center, const vec3 &up) noexcept {
		return {};
	}

	mat4 mat4::perspective(float fov_y, float aspect, float near_z, float far_z) noexcept {
		return {};
	}

	mat4 mat4::orthographic(float left, float right, float bottom, float top, float near_z, float far_z) noexcept {
		return {};
	}

	mat4 mat4::perspective_reverse_z(float fov_y, float aspect, float near_z) noexcept {
		return {};
	}

	mat4::mat4(const __m128 c0, const __m128 c1, const __m128 c2, const __m128 c3) noexcept
		: m_col {c0, c1, c2, c3} {}
}

