#include <iostream>
#include "Window.hpp"

#include "Application.hpp"


Window::Window(Application* app, HINSTANCE hInstance, float width, float height) : m_app(app)
{

	// Initialize the window class.
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WndProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.lpszClassName = L"DXSampleClass";
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	m_hwnd = CreateWindow(
		windowClass.lpszClassName,
		L"Zenyth Engine - DirectX 12",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,        // We have no parent window.
		nullptr,        // We aren't using menus.
		hInstance,
		m_app);

	ShowWindow(m_hwnd, SW_NORMAL);
}

LRESULT Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool sizeMove = false;
	auto* app = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (msg)
	{
		case WM_CREATE:
		{
			// Save the Application* passed in to CreateWindow.
			auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		return 0;
		case WM_KEYDOWN:
			app->OnKeyDown(static_cast<uint8_t>(wParam));
			return 0;

		case WM_KEYUP:
			app->OnKeyUp(static_cast<uint8_t>(wParam));
			return 0;

		case WM_PAINT:
			if (app)
			{
				app->OnUpdate();
				app->OnRender();
			}
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_ENTERSIZEMOVE:
				sizeMove = true;
			break;

		case WM_EXITSIZEMOVE:
			sizeMove = false;
			if (app) {
				RECT rc;
				GetClientRect(hWnd, &rc);

				app->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
			}
			break;

		default:
			break;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
