#pragma once

#include "stdafx.h"

class Application;


class Window
{

public:
	Window(Application* app, HINSTANCE hInstance, float width, float height);
	HWND GetHWND() { return m_hwnd; }

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	HWND m_hwnd;
	Application* m_app;
};