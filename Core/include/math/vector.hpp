#pragma once
#include <numbers>

namespace zenyth::math {
	inline constexpr float epsilon = std::numeric_limits<float>::epsilon();
	inline constexpr float PI = std::numbers::pi;
	inline constexpr float PI2 = PI * 2.0f;

	class vec2 {
	public:
		vec2(float x, float y);
		vec2(float scalar);
		vec2();

		[[nodiscard]] float x() const noexcept;
		[[nodiscard]] float y() const noexcept;

		[[nodiscard]] vec2 operator+(const vec2& other) const noexcept;
		[[nodiscard]] vec2 operator-(const vec2& other) const noexcept;
		[[nodiscard]] vec2 operator*(float scalar) const noexcept;
		[[nodiscard]] vec2 operator/(float scalar) const noexcept;

		void operator+=(const vec2& other) noexcept;
		void operator-=(const vec2& other) noexcept;
		void operator*=(float scalar) noexcept;
		void operator/=(float scalar) noexcept;

		[[nodiscard]] vec2 operator-() const noexcept;

		[[nodiscard]] float dot(const vec2& other) const noexcept;
		[[nodiscard]] float length_sq() const noexcept;
		[[nodiscard]] float length() const noexcept;
	protected:
		float m_x, m_y;
	};

	class vec3 {
	public:
		vec3(float x, float y, float z);
		vec3(float scalar);
		vec3();

		[[nodiscard]] float x() const noexcept;
		[[nodiscard]] float y() const noexcept;
		[[nodiscard]] float z() const noexcept;

		[[nodiscard]] vec3 operator+(const vec3& other) const noexcept;
		[[nodiscard]] vec3 operator-(const vec3& other) const noexcept;
		[[nodiscard]] vec3 operator*(float scalar) const noexcept;
		[[nodiscard]] vec3 operator/(float scalar) const noexcept;

		void operator+=(const vec3& other) noexcept;
		void operator-=(const vec3& other) noexcept;
		void operator*=(float scalar) noexcept;
		void operator/=(float scalar) noexcept;

		[[nodiscard]] vec3 operator-() const noexcept;

		[[nodiscard]] vec3 cross(const vec3& other) const noexcept;

		[[nodiscard]] float dot(const vec3& other) const noexcept;
		[[nodiscard]] float length_sq() const noexcept;
		[[nodiscard]] float length() const noexcept;
	protected:
		vec3(__m128 simd) : m_simd(simd) {};
		__m128 m_simd;
	};

	class vec4 {
	public:
		vec4(float x, float y, float z, float w);
		vec4(float scalar);
		vec4();

		[[nodiscard]] float x() const noexcept;
		[[nodiscard]] float y() const noexcept;
		[[nodiscard]] float z() const noexcept;
		[[nodiscard]] float w() const noexcept;

		[[nodiscard]] vec4 operator+(const vec4& other) const noexcept;
		[[nodiscard]] vec4 operator-(const vec4& other) const noexcept;
		[[nodiscard]] vec4 operator*(float scalar) const noexcept;
		[[nodiscard]] vec4 operator/(float scalar) const noexcept;

		[[nodiscard]] void operator+=(const vec4& other) noexcept;
		[[nodiscard]] void operator-=(const vec4& other) noexcept;
		[[nodiscard]] void operator*=(float scalar) noexcept;
		[[nodiscard]] void operator/=(float scalar) noexcept;

		[[nodiscard]] vec4 operator-() const noexcept;

		[[nodiscard]] float dot(const vec4& other) const noexcept;
		[[nodiscard]] float length_sq() const noexcept;
		[[nodiscard]] float length() const noexcept;
	private:
		vec4(__m128 simd) : m_simd(simd) {};
		__m128 m_simd;
	};
}


