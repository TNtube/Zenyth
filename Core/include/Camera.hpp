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
		float fov;
		float nearPlane = 0.01f;
		float farPlane = 1000.0f;

		Vector3 camPos = Vector3(0, 0, 0);
		Quaternion camRot = Quaternion::Identity;
		Matrix projection;
		Matrix view;

		int lastMouseX = 0;
		int lastMouseY = 0;
	public:
		Camera(float fov, float aspectRatio);
		~Camera();

		void UpdateAspectRatio(float aspectRatio);
		void Update(float dt, const Keyboard::State &kb, Mouse* mouse);

		void SetPosition(const Vector3& pos) { camPos = pos; }

		CameraData GetCameraData() const;
	};
}
