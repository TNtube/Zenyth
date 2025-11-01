#include "pch.hpp"

#include "Win32Application.hpp"
#include "ModelGallery.hpp"
#include "Renderer/Renderer.hpp"

#include <dxgidebug.h>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, const int nShowCmd)
{
	{
		ModelGallery app(1280, 720, false);
		Win32Application::Run(&app, hInstance, nShowCmd);
	}


	Microsoft::WRL::ComPtr<IDXGIDebug1> pDebug = nullptr;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(pDebug.GetAddressOf()))))
	{
		pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	}

	return 0;
}
