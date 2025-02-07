#include "stdafx.h"

#include "Application.hpp"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	Application app(hInstance);
	app.Run();
}