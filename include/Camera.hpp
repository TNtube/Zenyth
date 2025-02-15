#pragma once
#include <memory>
#include <directxtk12/SimpleMath.h>
#include <directxtk12/Mouse.h>
#include <directxtk12/Keyboard.h>

#include "Buffers.hpp"


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
		void Update(float dt, Keyboard::State kb, Mouse* mouse);

		void SetPosition(Vector3 pos) { camPos = pos; }

		CameraData GetCameraData() const;
	};
}
