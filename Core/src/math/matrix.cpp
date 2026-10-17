#include "pch.hpp"
#include "math/matrix.hpp"

namespace zenyth::math {
	mat4::mat4() noexcept {}
	mat4::mat4(const __m128 c0, const __m128 c1, const __m128 c2, const __m128 c3) noexcept
		: m_col {c0, c1, c2, c3} {}
	mat4::mat4(const vec4 &col0, const vec4 &col1, const vec4 &col2, const vec4 &col3) noexcept
		: m_col {col0.m_simd, col1.m_simd, col2.m_simd, col3.m_simd} {}

	mat4::mat4(const float diagonal) noexcept
		: m_col {
			_mm_set_ps(0, 0, 0, diagonal),  // col0: (1, 0, 0, 0)
			_mm_set_ps(0, 0, diagonal, 0),  // col1: (0, 1, 0, 0)
			_mm_set_ps(0, diagonal, 0, 0),  // col2: (0, 0, 1, 0)
			_mm_set_ps(diagonal, 0, 0, 0)   // col3: (0, 0, 0, 1)
			} {}

	mat4 mat4::identity() noexcept {
		return mat4{1};
	}

	mat4 mat4::zero() noexcept {
		return mat4{};
	}

	mat4 mat4::from_euler(const vec3& euler) noexcept {
		const float cx = std::cos(euler.x());
		const float sx = std::sin(euler.x());
		const float cy = std::cos(euler.y());
		const float sy = std::sin(euler.y());
		const float cz = std::cos(euler.z());
		const float sz = std::sin(euler.z());

		return {
			_mm_set_ps(0.0f, -sy, cy * sz, cy * cz), // col0
			_mm_set_ps(0.0f,  sx * cy, cx * cz + sx * sy * sz, -cx * sz + sx * sy * cz), // col1
			_mm_set_ps(0.0f,  cx * cy, -sx * cz + cx * sy * sz, sx * sz + cx * sy * cz), // col2
			_mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f) // col3
		};
	}

	mat4 mat4::from_scale(const vec3 &scale) noexcept {
		return {
			_mm_set_ps(0, 0, 0, scale.x()),     // col0: (sx, 0, 0, 0)
			_mm_set_ps(0, 0, scale.y(), 0),     // col1: (0, sy, 0, 0)
			_mm_set_ps(0, scale.z(), 0, 0),     // col2: (0, 0, sz, 0)
			_mm_set_ps(1, 0, 0, 0)              // col3: (0, 0, 0, 1)
		};
	}

	mat4 mat4::from_translation(const vec3& t) noexcept {
		return {
			_mm_set_ps(0, 0, 0, 1),             // col0: (1, 0, 0, 0)
			_mm_set_ps(0, 0, 1, 0),             // col1: (0, 1, 0, 0)
			_mm_set_ps(0, 1, 0, 0),             // col2: (0, 0, 1, 0)
			_mm_set_ps(1, t.z(), t.y(), t.x())  // col3: (tx, ty, tz, 1)
		};
	}

	__m128 mat4::linear_combine(const __m128 (&cols)[4], __m128 r) noexcept {
		return _mm_add_ps(
			_mm_add_ps(
				_mm_mul_ps(cols[0], _mm_shuffle_ps(r, r, _MM_SHUFFLE(0,0,0,0))),
				_mm_mul_ps(cols[1], _mm_shuffle_ps(r, r, _MM_SHUFFLE(1,1,1,1)))
			),
			_mm_add_ps(
				_mm_mul_ps(cols[2], _mm_shuffle_ps(r, r, _MM_SHUFFLE(2,2,2,2))),
				_mm_mul_ps(cols[3], _mm_shuffle_ps(r, r, _MM_SHUFFLE(3,3,3,3)))
			)
		);
	}

	mat4 mat4::operator*(const mat4& rhs) const noexcept {
		mat4 result;
		for (int i = 0; i < 4; ++i)
			result.m_col[i] = linear_combine(m_col, rhs.m_col[i]);
		return result;
	}

	mat4& mat4::operator*=(const mat4 &rhs) noexcept {
		const __m128 lhs[4] = { m_col[0], m_col[1], m_col[2], m_col[3] };
		for (int i = 0; i < 4; ++i)
			m_col[i] = linear_combine(lhs, rhs.m_col[i]);
		return *this;
	}

	vec4 mat4::operator*(const vec4 &v) const noexcept {
		return vec4{ linear_combine(m_col, v.m_simd) };
	}

	mat4 mat4::look_at(const vec3& eye, const vec3& center, const vec3& up) noexcept {
		// Forward: from eye toward center (right-handed: negative Z)
		vec3 forward = vec3::normalize(center - eye);
		// Right: perpendicular to forward and up
		vec3 right = forward.cross(up).normalize();
		// Recompute up to guarantee orthonormality
		vec3 _up = vec3::cross(right, forward);

		return {
			_mm_set_ps(0, -forward.x(), _up.x(), right.x()),               // col0: (right.x,  up.x,  -fwd.x, 0)
			_mm_set_ps(0, -forward.y(), _up.y(), right.y()),               // col1: (right.y,  up.y,  -fwd.y, 0)
			_mm_set_ps(0, -forward.z(), _up.z(), right.z()),               // col2: (right.z,  up.z,  -fwd.z, 0)
			_mm_set_ps(1, forward.dot(eye), -up.dot(eye), -right.dot(eye)) // col3: (-dot(r,eye), -dot(u,eye), dot(f,eye), 1)
		};
	}

	mat4 mat4::perspective(const float fov_y, const float aspect, const float near_z, const float far_z) noexcept {
		const float f     = 1.0f / std::tan(fov_y * 0.5f);
		const float range = near_z - far_z;

		return {
			_mm_set_ps(0, 0, 0, f / aspect) ,                  // col0
			_mm_set_ps(0, 0, f, 0),                            // col1
			_mm_set_ps(-1, far_z / range, 0, 0),               // col2
			_mm_set_ps(0, (near_z * far_z) / range, 0, 0)      // col3
		};
	}
}
