#include <iostream>
#include "Application.hpp"


void Application::Run()
{

	// Parse the command line parameters
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	// do something with the arguments

	LocalFree(argv);

}

Application::Application(HINSTANCE hInstance) : m_window(this, hInstance, 1280, 720)
{

}

void Application::OnUpdate()
{

}

void Application::OnRender()
{

}

void Application::OnKeyDown(uint8_t key)
{
	std::cout << "Key Down: " << key << std::endl;
}

void Application::OnKeyUp(uint8_t key)
{
	std::cout << "Key Up: " << key << std::endl;
}
