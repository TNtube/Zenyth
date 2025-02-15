#pragma once
#include <SimpleMath.h>

namespace Zenyth
{
	using namespace  DirectX::SimpleMath;

	class Transform
	{
	public:
		Transform();

		void SetEulerAngles(float x, float y, float z);
		void SetEulerAngles(Vector3 angles) { SetEulerAngles(angles.x, angles.y, angles.z); }

		void SetPosition(float x, float y, float z);
		void SetPosition(const Vector3 position) { SetPosition(position.x, position.y, position.z); }

		void SetScale(float x, float y, float z);
		void SetScale(const Vector3 scale) { SetScale(scale.x, scale.y, scale.z); }

		Vector3 GetForward() const;
		Vector3 GetRight() const;
		Vector3 GetUp() const;

		Vector3 GetPosition() const { return m_position; }
		Quaternion GetRotation() const { return m_rotation; }
		Vector3 GetScale() const { return m_scale; }

		Matrix GetTransformMatrix() const { return m_transform; }

	private:
		void UpdateTransform();
		Vector3			m_position;
		Quaternion		m_rotation;
		Vector3			m_scale;

		Matrix			m_transform;
	};
}
