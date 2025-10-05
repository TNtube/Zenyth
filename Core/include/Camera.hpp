#pragma once

#include <Mouse.h>
#include <Keyboard.h>

#include "StepTimer.hpp"


namespace Zenyth
{
	using namespace DirectX;
	using namespace DirectX::SimpleMath;

	// Sent to pixel shader
	struct CameraData {
		Matrix mViewProjection;
	};

	class Camera {
	public:
		Camera(float fov, float aspectRatio);
		~Camera();

		void UpdateAspectRatio(float aspectRatio);
		void Update(float dt, const Keyboard::State &kb, Mouse* mouse);
		void UpdateView();

		const Vector3& GetPosition() const { return m_camPos; }

		void SetPosition(const Vector3& pos) { m_camPos = pos; }
		void SetRotation(const Quaternion& rot) { m_camRot = rot; }

		[[nodiscard]] CameraData GetCameraData() const;
	private:
		float m_fov;
		float m_nearPlane = 0.01f;
		float m_farPlane = 1000.0f;

		Vector3 m_camPos = Vector3(0, 0, 0);
		Quaternion m_camRot = Quaternion::Identity;
		Matrix m_projection;
		Matrix m_view;
	};
}
