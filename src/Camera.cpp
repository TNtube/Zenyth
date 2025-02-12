#include "Camera.hpp"
#include "directxtk12/SimpleMath.h"

namespace Zenyth
{
	using namespace DirectX::SimpleMath;

	Camera::Camera(float fov, float aspectRatio) : fov(fov) {
		Vector3 forward = Vector3::Transform(Vector3::Forward, camRot);
		view = Matrix::CreateLookAt(camPos, camPos + forward, Vector3::Up);

		projection = Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, nearPlane, farPlane);
	}

	Camera::~Camera() = default;

	void Camera::UpdateAspectRatio(float aspectRatio) {
		projection = Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, nearPlane, farPlane);
	}

	void Camera::Update(float dt, Keyboard::State kb, Mouse* mouse) {
		float camSpeedRot = 0.25f;
		float camSpeedMouse = 10.0f;
		float camSpeed = 15.0f;
		if (kb.LeftShift) camSpeed *= 2.0f;

		Mouse::State mstate = mouse->GetState();
		const Matrix im = view.Invert();

		Vector3 delta;
		// TP: deplacement par clavier
		if (kb.Z) delta += Vector3::Forward;
		if (kb.S) delta += Vector3::Backward;
		if (kb.Q) delta += Vector3::Left;
		if (kb.D) delta += Vector3::Right;
		if (kb.A) delta += Vector3::Down;
		if (kb.E) delta += Vector3::Up;

		camPos += Vector3::TransformNormal(delta, im) * camSpeed * dt;

		// astuce: Vector3::TransformNormal(vecteur, im); transforme un vecteur de l'espace cameravers l'espace monde

		if (mstate.positionMode == Mouse::MODE_RELATIVE) {
			float dx = mstate.x;
			float dy = mstate.y;

			if (mstate.rightButton) {
				camPos += Vector3::TransformNormal(Vector3::Right, im) * -dx * camSpeedMouse * dt;
				camPos += Vector3::TransformNormal(Vector3::Up, im) * dy * camSpeedMouse * dt;

			} else if (mstate.leftButton) {
				camRot *= Quaternion::CreateFromAxisAngle(Vector3::TransformNormal(Vector3::Up, im), -dx * camSpeedRot * dt);
				camRot *= Quaternion::CreateFromAxisAngle(Vector3::TransformNormal(Vector3::Right, im), -dy * camSpeedRot * dt);
			} else {
				mouse->SetMode(Mouse::MODE_ABSOLUTE);
			}
		} else if (mstate.rightButton || mstate.leftButton) {
			mouse->SetMode(Mouse::MODE_RELATIVE);
		}

		Vector3 forward = Vector3::Transform(Vector3::Forward, camRot);
		view = Matrix::CreateLookAt(camPos, camPos + forward, Vector3::Up);
	}

	CameraData Camera::GetCameraData() const {

		CameraData md;
		md.mView = view.Transpose();
		md.mProjection = projection.Transpose();
		md.camPos = Vector4(camPos);
		md.camPos.w = 1.0f;
		return std::move(md);
	}
}