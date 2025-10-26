#include "pch.hpp"

#include "Application.hpp"

#include "Renderer/Renderer.hpp"

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

namespace Zenyth
{
	Application* Application::s_App = nullptr;

	_Use_decl_annotations_
	Application::Application(const uint32_t width, const uint32_t height, const bool useWrapDevice) :
		m_width(width),
		m_height(height),
		m_useWarpDevice(useWrapDevice),
		m_renderer(useWrapDevice)
	{
		assert(!s_App);
		s_App = this;

		m_renderer.Init();
	}

}
