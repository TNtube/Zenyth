#include "pch.hpp"
#include "Camera.hpp"

namespace Zenyth
{
	using namespace DirectX::SimpleMath;

	Camera::Camera(float fov, float aspectRatio) : m_fov(fov) {
		Vector3 forward = Vector3::Transform(Vector3::Forward, m_camRot);
		m_view = Matrix::CreateLookAt(m_camPos, m_camPos + forward, Vector3::Up);

		m_projection = Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, m_nearPlane, m_farPlane);
	}

	Camera::~Camera() = default;

	void Camera::UpdateAspectRatio(const float aspectRatio) {
		m_projection = Matrix::CreatePerspectiveFieldOfView(m_fov, aspectRatio, m_nearPlane, m_farPlane);
	}

	void Camera::Update(const float dt, const Keyboard::State &kb, Mouse* mouse) {
		float camSpeed = 15.0f;
		if (kb.LeftShift) camSpeed *= 2.0f;

		const Mouse::State mstate = mouse->GetState();
		const Matrix im = m_view.Invert();

		Vector3 delta;
		// TP: deplacement par clavier
		if (kb.Z) delta += Vector3::Forward;
		if (kb.S) delta += Vector3::Backward;
		if (kb.Q) delta += Vector3::Left;
		if (kb.D) delta += Vector3::Right;
		if (kb.A) delta += Vector3::Down;
		if (kb.E) delta += Vector3::Up;

		m_camPos += Vector3::TransformNormal(delta, im) * camSpeed * dt;

		// astuce: Vector3::TransformNormal(vecteur, im); transforme un vecteur de l'espace cameravers l'espace monde

		if (mstate.positionMode == Mouse::MODE_RELATIVE) {
			const float dx = mstate.x;
			const float dy = mstate.y;

			if (mstate.rightButton) {
				constexpr float camSpeedMouse = 10.0f;

				m_camPos += Vector3::TransformNormal(Vector3::Right, im) * -dx * camSpeedMouse * dt;
				m_camPos += Vector3::TransformNormal(Vector3::Up, im) * dy * camSpeedMouse * dt;

			} else if (mstate.leftButton) {
				constexpr float camSpeedRot = 0.25f;

				m_camRot *= Quaternion::CreateFromAxisAngle(Vector3::TransformNormal(Vector3::Up, im), -dx * camSpeedRot * dt);
				m_camRot *= Quaternion::CreateFromAxisAngle(Vector3::TransformNormal(Vector3::Right, im), -dy * camSpeedRot * dt);
			} else {
				mouse->SetMode(Mouse::MODE_ABSOLUTE);
			}
		} else if (mstate.rightButton || mstate.leftButton) {
			mouse->SetMode(Mouse::MODE_RELATIVE);
		}

		const Vector3 forward = Vector3::Transform(Vector3::Forward, m_camRot);
		m_view = Matrix::CreateLookAt(m_camPos, m_camPos + forward, Vector3::Up);
	}

	CameraData Camera::GetCameraData() const {

		CameraData md;
		md.mView = m_view.Transpose();
		md.mProjection = m_projection.Transpose();
//		md.camPos = Vector4(camPos);
//		md.camPos.w = 1.0f;
		return md;
	}
}