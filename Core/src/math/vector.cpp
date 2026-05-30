#include "pch.hpp"
#include "math/vector.hpp"

#include <immintrin.h>

namespace zenyth::math {
	vec4::vec4(float x, float y, float z, float w)
	{
		m_simd = _mm_set_ps(w, z, y, x);
	}

	vec4::vec4(float scalar)
	{
		m_simd = _mm_set1_ps(scalar);
	}

	vec4 vec4::operator+(const vec4& other) const
	{
		return { _mm_add_ps(m_simd, other.m_simd) };
	}

	void vec4::operator+=(const vec4& other)
	{
		m_simd = _mm_add_ps(m_simd, other.m_simd);
	}

	vec4 vec4::operator-(const vec4& other) const
	{
		return { _mm_sub_ps(m_simd, other.m_simd) };
	}

	void vec4::operator-=(const vec4& other)
	{
		m_simd = _mm_sub_ps(m_simd, other.m_simd);
	}

	vec4 vec4::operator*(const vec4& other) const
	{
		return { _mm_mul_ps(m_simd, other.m_simd) };
	}

	void vec4::operator*=(const vec4& other)
	{
		m_simd = _mm_mul_ps(m_simd, other.m_simd);
	}

	vec4 vec4::operator/(const vec4& other) const
	{
		return { _mm_div_ps(m_simd, other.m_simd) };
	}

	void vec4::operator/=(const vec4& other)
	{
		m_simd = _mm_div_ps(m_simd, other.m_simd);
	}

	vec4 vec4::operator-()
	{
		return { _mm_sub_ps(_mm_setzero_ps(), m_simd) };
	}
	
	float vec4::dot(const vec4& other) const
	{
		return _mm_cvtss_f32(_mm_dp_ps(m_simd, other.m_simd, 0xF1));
	}

	float vec4::length_sq() const
	{
		return dot(*this);
	}

	float vec4::length() const
	{
		return std::sqrtf(length_sq());
	}
}