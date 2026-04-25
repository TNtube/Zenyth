#include <iostream>

#include "SandboxApp.hpp"
#include "D3D12Renderer.hpp"



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, const int nShowCmd)
{
	SandboxApp app;

	std::unique_ptr<Zenyth::IRenderer> renderer = std::make_unique<Zenyth::D3D12Renderer>();
	app.SetRenderer(std::move(renderer));

	app.Run();

	return 0;
}
