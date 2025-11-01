#include "pch.hpp"
#include "Data/Transform.hpp"


Transform::Transform() : m_scale(1, 1, 1)
{
	m_Dirty = true;
}

void Transform::SetEulerAngles(const float x, const float y, const float z)
{
	m_Dirty = true;
	m_rotation = Quaternion::CreateFromYawPitchRoll(y, x, z);
}

void Transform::SetPosition(const float x, const float y, const float z)
{
	m_Dirty = true;
	m_position = Vector3{ x, y, z };
}

void Transform::SetScale(const float x, const float y, const float z)
{
	m_Dirty = true;
	m_scale = Vector3{ x, y, z };
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

Matrix Transform::GetTransformMatrix()
{
	if (m_Dirty) UpdateTransform();
	return m_transform;
}

void Transform::UpdateTransform()
{
	m_transform = Matrix::CreateScale(m_scale) * Matrix::CreateFromQuaternion(m_rotation) * Matrix::CreateTranslation(m_position);
}
