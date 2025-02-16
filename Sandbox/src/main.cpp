#include "pch.hpp"

#include "Application.hpp"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	Application app(hInstance, false);
	app.Run();

	return 0;
}
