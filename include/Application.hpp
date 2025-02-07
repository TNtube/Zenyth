#pragma once

#include "Window.hpp"

class Application
{
public:
	Application(HINSTANCE hInstance);
	void Run();

	void OnKeyDown(uint8_t key);
	void OnKeyUp(uint8_t key);

	void OnUpdate();
	void OnRender();
private:

	Window m_window;
};