#include "engine/Transform.hpp"

namespace Zenyth
{
	Transform::Transform() : m_scale(1, 1, 1)
	{

	}

	void Transform::SetEulerAngles(float x, float y, float z)
	{
		m_rotation = Quaternion::CreateFromYawPitchRoll(y, x, z);
		UpdateTransform();
	}

	void Transform::SetPosition(float x, float y, float z)
	{
		m_position = Vector3{ x, y, z };
		UpdateTransform();
	}

	void Transform::SetScale(float x, float y, float z)
	{
		m_scale = Vector3{ x, y, z };
		UpdateTransform();
	}

	Vector3 Transform::GetForward() const
	{
		return Vector3::Transform(Vector3::Forward, m_rotation);
	}

	Vector3 Transform::GetRight() const
	{
		return Vector3::Transform(Vector3::Right, m_rotation);
	}

	Vector3 Transform::GetUp() const
	{
		return Vector3::Transform(Vector3::Up, m_rotation);
	}

	void Transform::UpdateTransform()
	{
		m_transform = Matrix::CreateScale(m_scale) * Matrix::CreateFromQuaternion(m_rotation) * Matrix::CreateTranslation(m_position);
	}
}
