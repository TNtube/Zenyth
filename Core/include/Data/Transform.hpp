#pragma once

namespace Zenyth
{
	using namespace  DirectX::SimpleMath;

	class Transform
	{
	public:
		Transform();

		void SetEulerAngles(float x, float y, float z);
		void SetEulerAngles(const Vector3& angles) { SetEulerAngles(angles.x, angles.y, angles.z); }

		void SetPosition(float x, float y, float z);
		void SetPosition(const Vector3& position) { SetPosition(position.x, position.y, position.z); }

		void SetScale(float x, float y, float z);
		void SetScale(const Vector3& scale) { SetScale(scale.x, scale.y, scale.z); }

		[[nodiscard]] Vector3 GetForward() const;
		[[nodiscard]] Vector3 GetRight() const;
		[[nodiscard]] Vector3 GetUp() const;

		[[nodiscard]] Vector3 GetPosition() const { return m_position; }
		[[nodiscard]] Quaternion GetRotation() const { return m_rotation; }
		[[nodiscard]] Vector3 GetScale() const { return m_scale; }

		[[nodiscard]] bool IsDirty() const { return m_Dirty; }

		Matrix GetTransformMatrix();

	private:
		void UpdateTransform();

		Vector3			m_position;
		Quaternion		m_rotation;
		Vector3			m_scale;

		Matrix			m_transform;

		bool m_Dirty = false;
	};
}
