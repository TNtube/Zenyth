#include "pch.hpp"
#include "Camera.hpp"

#include "Math/Utils.hpp"

using namespace DirectX::SimpleMath;
using namespace DirectX;

Camera::Camera(float fov, float aspectRatio) : m_fov(fov) {
	Vector3 forward = Vector3::Transform(Vector3::Forward, m_camRot);
	m_view = Matrix::CreateLookAt(m_camPos, m_camPos + forward, Vector3::Up);

	UpdateAspectRatio(aspectRatio);
}

Camera::~Camera() = default;

void Camera::UpdateAspectRatio(const float aspectRatio) {
	m_projection = Matrix::CreatePerspectiveFieldOfView(m_fov, aspectRatio, m_nearPlane, m_farPlane);
	m_projection = Math::ApplyReverseZ(m_projection);
}

void Camera::Update(const float dt, const Keyboard::State& kb, Mouse* mouse) {
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
		const auto dx = static_cast<float>(mstate.x);
		const auto dy = static_cast<float>(mstate.y);

		if (mstate.rightButton) {
			Vector3 deltaMouse;
			if(kb.LeftShift || kb.RightShift)
				deltaMouse = Vector3(0, 0, dy);
			else
				deltaMouse = Vector3(-dx, dy, 0);
			m_camPos += Vector3::TransformNormal(deltaMouse, im) * camSpeed * dt;

		} else if (mstate.leftButton) {
			constexpr float camSpeedRot = 0.25f;
			m_camRot *= Quaternion::CreateFromAxisAngle(Vector3::TransformNormal(Vector3::Right, im), -dy * dt * camSpeedRot);
			m_camRot *= Quaternion::CreateFromAxisAngle(Vector3::Up, -dx * dt * camSpeedRot);
		} else {
			mouse->SetMode(Mouse::MODE_ABSOLUTE);
		}
	} else if (mstate.rightButton || mstate.leftButton) {
		mouse->SetMode(Mouse::MODE_RELATIVE);
	}
	UpdateView();
}

void Camera::UpdateView()
{
	const Vector3 forward = Vector3::Transform(Vector3::Forward, m_camRot);
	const Vector3 up = Vector3::Transform(Vector3::Up, m_camRot);
	m_view = Matrix::CreateLookAt(m_camPos, m_camPos + forward, up);
}

CameraData Camera::GetCameraData() const {

	CameraData md {};
	md.mViewProjection = m_view * m_projection;
	return md;
}
