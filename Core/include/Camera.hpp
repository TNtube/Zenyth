#pragma once

#include <Mouse.h>
#include <Keyboard.h>


namespace Zenyth
{
	using namespace DirectX;
	using namespace DirectX::SimpleMath;

	struct CameraData {
		Matrix mView;
		Matrix mProjection;
	};

	class Camera {
	public:
		Camera(float fov, float aspectRatio);
		~Camera();

		void UpdateAspectRatio(float aspectRatio);
		void Update(float dt, const Keyboard::State &kb, Mouse* mouse);

		void SetPosition(const Vector3& pos) { m_camPos = pos; }

		CameraData GetCameraData() const;
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
