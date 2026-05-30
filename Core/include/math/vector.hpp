#pragma once


namespace zenyth::math {
	class vec4 {
	public:
		vec4(float x, float y, float z, float w);
		vec4(float scalar);

		vec4 operator+(const vec4& other) const;
		void operator+=(const vec4& other);
		vec4 operator-(const vec4& other) const;
		void operator-=(const vec4& other);
		vec4 operator*(const vec4& other) const;
		void operator*=(const vec4& other);
		vec4 operator/(const vec4& other) const;
		void operator/=(const vec4& other);

		vec4 operator-();

		float dot(const vec4& other) const;
		float length_sq() const;
		float length() const;
	private:
		vec4(__m128 simd) : m_simd(simd) {};
		__m128 m_simd;
	};
}


