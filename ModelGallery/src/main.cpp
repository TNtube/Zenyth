#include "pch.hpp"

#include "Win32Application.hpp"
#include "ModelGallery.hpp"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, const int nShowCmd)
{
	ModelGallery app(1280, 720, false);
	Zenyth::Win32Application::Run(&app, hInstance, nShowCmd);

	return 0;
}
