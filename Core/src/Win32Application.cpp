#include "pch.hpp"

#include "Core.hpp"

#include "Win32Application.hpp"
#include "Mouse.h"
#include "Keyboard.h"


#include <shlobj.h>
#include <strsafe.h>

#include <backends/imgui_impl_win32.h>
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
std::wstring GetLatestWinPixGpuCapturerPath();

HWND Win32Application::m_hwnd = nullptr;

int Win32Application::Run(Application* pApp, HINSTANCE hInstance, const int nCmdShow)
{
	// Parse the command line parameters
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	if (argc > 1 && std::wcscmp(argv[1], L"pix") == 0)
	{
		if (GetModuleHandle(L"WinPixGpuCapturer.dll") == nullptr)
		{
			const std::wstring winPixGpuCapturerPath = GetLatestWinPixGpuCapturerPath();
			if (!winPixGpuCapturerPath.empty())
			{
				LoadLibrary(winPixGpuCapturerPath.c_str());
			}
		}
	}

	LocalFree(argv);

	ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

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

	CoUninitialize();

	// Return this part of the WM_QUIT message to Windows.
	return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;
	auto* pApp = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (msg)
	{
		case WM_CREATE:
		{
			// Save the Application* passed in to CreateWindow.
			const auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		return 0;
		case WM_PAINT:
			pApp->OnUpdate();
			pApp->OnRender();
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
		{
			const auto& io = ImGui::GetIO();
			if (!io.WantCaptureMouse)
				DirectX::Mouse::ProcessMessage(msg, wParam, lParam);
		}
		break;

		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		case WM_SYSKEYDOWN:
			DirectX::Keyboard::ProcessMessage(msg, wParam, lParam);
		break;

		case WM_EXITSIZEMOVE:
		if (pApp) {
			RECT rc;
			GetClientRect(hWnd, &rc);

			pApp->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
		}
		break;

		case WM_SIZE:
		if (pApp)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);

			pApp->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
		}

		default:
			break;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, msg, wParam, lParam);
}


static std::wstring GetLatestWinPixGpuCapturerPath()
{
	LPWSTR programFilesPath = nullptr;
	SUCCEEDED(SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, nullptr, &programFilesPath));

	const std::wstring pixSearchPath = programFilesPath + std::wstring(L"\\Microsoft PIX\\*");

	WIN32_FIND_DATA findData;
	bool foundPixInstallation = false;
	wchar_t newestVersionFound[MAX_PATH];

	void* const hFind = FindFirstFile(pixSearchPath.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) &&
				 (findData.cFileName[0] != '.'))
			{
				if (!foundPixInstallation || wcscmp(newestVersionFound, findData.cFileName) <= 0)
				{
					foundPixInstallation = true;
					SUCCEEDED(StringCchCopy(newestVersionFound, _countof(newestVersionFound), findData.cFileName));
				}
			}
		}
		while (FindNextFile(hFind, &findData) != 0);
	}

	FindClose(hFind);

	if (!foundPixInstallation)
	{
		return L"";
	}

	wchar_t output[MAX_PATH];
	SUCCEEDED(StringCchCopy(output, pixSearchPath.length(), pixSearchPath.data()));
	SUCCEEDED(StringCchCat(output, MAX_PATH, &newestVersionFound[0]));
	SUCCEEDED(StringCchCat(output, MAX_PATH, L"\\WinPixGpuCapturer.dll"));

	return &output[0];
}