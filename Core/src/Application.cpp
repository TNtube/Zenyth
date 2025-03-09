#include "pch.hpp"

#include "Application.hpp"

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

namespace Zenyth
{

	_Use_decl_annotations_
	Application::Application(const uint32_t width, const uint32_t height, const bool useWrapDevice) :
		m_width(width),
		m_height(height),
		m_useWarpDevice(useWrapDevice)
	{
	}

}
