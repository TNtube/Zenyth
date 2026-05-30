#include <iostream>

#include "SandboxApp.hpp"
#include "D3D12Renderer.hpp"
#include "math/vector.hpp"



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, const int nShowCmd)
{
	zenyth::math::vec4 vec{ 1, 2, 3, 4 };

	SandboxApp app;

	std::unique_ptr<Zenyth::IRenderer> renderer = std::make_unique<Zenyth::D3D12Renderer>();
	app.SetRenderer(std::move(renderer));

	app.Run();

	return 0;
}
