#include "SandboxApp.hpp"
#include "D3D12Renderer.hpp"
#include "math/vector.hpp"
#include "math/matrix.hpp"



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, const int nShowCmd)
{
	SandboxApp app;

	std::unique_ptr<zenyth::renderer::IRenderer> renderer = std::make_unique<zenyth::renderer::D3D12Renderer>();
	app.SetRenderer(std::move(renderer));

	app.Run();

	return 0;
}
