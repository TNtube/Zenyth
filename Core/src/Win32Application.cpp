#include "pch.hpp"

#include "Win32Application.hpp"
#include "Mouse.h"
#include "Keyboard.h"

namespace Zenyth
{
	HWND Win32Application::m_hwnd = nullptr;

	int Win32Application::Run(Application* pApp, const HINSTANCE hInstance, const int nCmdShow)
	{
		// Parse the command line parameters
		int argc;
		LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

		LocalFree(argv);

		WNDCLASSEX windowClass = { 0 };
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = WindowProc;
		windowClass.hInstance = hInstance;
		windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		windowClass.lpszClassName = L"ZenythWindowClass";
		RegisterClassEx(&windowClass);

		RECT windowRect = { 0, 0, static_cast<LONG>(pApp->GetWidth()), static_cast<LONG>(pApp->GetHeight()) };
		AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

		m_hwnd = CreateWindow(
			windowClass.lpszClassName,
			L"Zenyth",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			nullptr,
			nullptr,
			hInstance,
			pApp);

		pApp->OnInit();

		ShowWindow(m_hwnd, nCmdShow);

		// Main loop.
		MSG msg = {};
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		pApp->OnDestroy();

		// Return this part of the WM_QUIT message to Windows.
		return static_cast<char>(msg.wParam);
	}

	// Main message handler for the sample.
	LRESULT CALLBACK Win32Application::WindowProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
	{
		auto* pApp = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		switch (message)
		{
			case WM_CREATE:
			{
				// Save the Application* passed in to CreateWindow.
				const auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
			}
			return 0;
			case WM_PAINT:
				if (pApp)
					pApp->Tick();
			return 0;

			case WM_DESTROY:
				PostQuitMessage(0);
			return 0;

			case WM_ACTIVATE:
			case WM_INPUT:
			case WM_MOUSEMOVE:
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_MOUSEWHEEL:
			case WM_XBUTTONDOWN:
			case WM_XBUTTONUP:
			case WM_MOUSEHOVER:
				DirectX::Mouse::ProcessMessage(message, wParam, lParam);
			break;

			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP:
			case WM_SYSKEYDOWN:
				DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
			break;

			case WM_EXITSIZEMOVE:
			if (pApp) {
				RECT rc;
				GetClientRect(hWnd, &rc);

				pApp->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
			}
			break;

			default:
				break;
		}

		// Handle any messages the switch statement didn't.
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
