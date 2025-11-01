#pragma once

#include <Mouse.h>
#include <Keyboard.h>

#include "StepTimer.hpp"


// Sent to pixel shader
struct CameraData {
	DirectX::SimpleMath::Matrix mViewProjection;
};

class Camera {
public:
	Camera(float fov, float aspectRatio);
	~Camera();

	void UpdateAspectRatio(float aspectRatio);
	void Update(float dt, const DirectX::Keyboard::State &kb, DirectX::Mouse* mouse);
	void UpdateView();

	const DirectX::SimpleMath::Vector3& GetPosition() const { return m_camPos; }

	void SetPosition(const DirectX::SimpleMath::Vector3& pos) { m_camPos = pos; }
	void SetRotation(const DirectX::SimpleMath::Quaternion& rot) { m_camRot = rot; }

	[[nodiscard]] CameraData GetCameraData() const;
private:
	float m_fov;
	float m_nearPlane = 0.01f;
	float m_farPlane = 1000.0f;

	DirectX::SimpleMath::Vector3 m_camPos = DirectX::SimpleMath::Vector3(0, 0, 0);
	DirectX::SimpleMath::Quaternion m_camRot = DirectX::SimpleMath::Quaternion::Identity;
	DirectX::SimpleMath::Matrix m_projection;
	DirectX::SimpleMath::Matrix m_view;
};
