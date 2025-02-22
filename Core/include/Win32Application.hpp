#pragma once

#include "Application.hpp"

namespace Zenyth
{
	class Win32Application
	{
	public:
		static int Run(Application* pApp, HINSTANCE hInstance, int nCmdShow);
		static HWND GetHwnd() { return m_hwnd; }

	protected:
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		static HWND m_hwnd;
	};
}
