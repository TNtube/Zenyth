#pragma once

namespace zenyth::math {
	class vec2 {
	public:
		explicit vec2(float scalar);
		vec2(float x, float y);
		vec2();

		[[nodiscard]] float x() const noexcept;
		[[nodiscard]] float y() const noexcept;

		float& x() noexcept;
		float& y() noexcept;

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
		explicit vec3(float scalar);
		vec3(float x, float y, float z);
		vec3();

		[[nodiscard]] float x() const noexcept;
		[[nodiscard]] float y() const noexcept;
		[[nodiscard]] float z() const noexcept;

		float& x() noexcept;
		float& y() noexcept;
		float& z() noexcept;

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
		explicit vec3(const __m128 simd) : m_simd(simd) {};

		union {
			struct { float m_x, m_y, m_z, m_w; } m_components{};
			__m128 m_simd;
		};
	};

	class vec4 {
	public:
		explicit vec4(float scalar);
		vec4(float x, float y, float z, float w);
		vec4();

		[[nodiscard]] float x() const noexcept;
		[[nodiscard]] float y() const noexcept;
		[[nodiscard]] float z() const noexcept;
		[[nodiscard]] float w() const noexcept;

		float& x() noexcept;
		float& y() noexcept;
		float& z() noexcept;
		float& w() noexcept;

		[[nodiscard]] vec4 operator+(const vec4& other) const noexcept;
		[[nodiscard]] vec4 operator-(const vec4& other) const noexcept;
		[[nodiscard]] vec4 operator*(float scalar) const noexcept;
		[[nodiscard]] vec4 operator/(float scalar) const noexcept;

		void operator+=(const vec4& other) noexcept;
		void operator-=(const vec4& other) noexcept;
		void operator*=(float scalar) noexcept;
		void operator/=(float scalar) noexcept;

		[[nodiscard]] vec4 operator-() const noexcept;

		[[nodiscard]] float dot(const vec4& other) const noexcept;
		[[nodiscard]] float length_sq() const noexcept;
		[[nodiscard]] float length() const noexcept;
	private:
		friend class mat4;
		explicit vec4(const __m128 simd) : m_simd(simd) {};

		union {
			struct { float m_x, m_y, m_z, m_w; } m_components{};
			__m128 m_simd;
		};
	};
} // namespace zenyth::math
