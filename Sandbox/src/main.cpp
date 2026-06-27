#include "SandboxApp.hpp"
#include "D3D12Renderer.hpp"
#include "math/vector.hpp"
#include "math/matrix.hpp"



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, const int nShowCmd)
{
	zenyth::math::vec4 vec{ 1, 2, 3, 4 };
	zenyth::math::mat4 mat;
	const float foo = mat[0, 0];

	std::stringstream ss;
	ss << foo << "\n";

	OutputDebugStringA(ss.str().c_str());

	SandboxApp app;

	std::unique_ptr<Zenyth::IRenderer> renderer = std::make_unique<Zenyth::D3D12Renderer>();
	app.SetRenderer(std::move(renderer));

	app.Run();

	return 0;
}
