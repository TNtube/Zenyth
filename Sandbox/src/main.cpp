#include "pch.hpp"

#include "Win32Application.hpp"
#include "Minicraft.hpp"


int WINAPI WinMain(const HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, const int nShowCmd)
{
	Minicraft app(1280, 720, false);
	Zenyth::Win32Application::Run(&app, hInstance, nShowCmd);

	return 0;
}
