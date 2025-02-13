#pragma once

#include "stdafx.h"

#include "Buffers.hpp"
#include "Camera.hpp"
#include "Window.hpp"
#include "Vertex.hpp"
#include "Texture.hpp"
#include "StepTimer.h"
#include "engine/Transform.hpp"

namespace Zenyth {
	struct CameraData;
}

using namespace DirectX;
using Microsoft::WRL::ComPtr;

constexpr uint32_t ScreenWidth = 1280;
constexpr uint32_t ScreenHeight = 720;

class Application
{
public:
	Application(HINSTANCE hInstance, bool useWrapDevice);
	~Application();
	void Run();

	void OnInit();
	void Tick();
	void OnUpdate();
	void OnRender();
	void OnDestroy();

	void OnWindowSizeChanged(int width, int height);
private:
	std::unique_ptr<Window> m_window{};

	static constexpr UINT FrameCount = 2;

	bool m_useWarpDevice = false;

	float m_aspectRatio;


	struct SceneConstantBuffer
	{
		SimpleMath::Matrix model;
		SimpleMath::Vector4 offset[12];
	};

	ComPtr<ID3D12Debug> debug;
	ComPtr<ID3D12Debug1> dxgiDebug;
	ComPtr<ID3D12InfoQueue> dxgiInfoQueue;

	DWORD m_callbackCookie{};

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_resourceHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	UINT m_rtvDescriptorSize;


	// App resources.
	std::unique_ptr<Zenyth::VertexBuffer<Vertex>> m_vertexBuffer{};
	std::unique_ptr<Zenyth::IndexBuffer> m_indexBuffer{};
	std::unique_ptr<Zenyth::Texture> m_texture{};
	Zenyth::Transform m_faceTransform {};
	std::unique_ptr<Zenyth::ConstantBuffer<SceneConstantBuffer>> m_constantBuffer{};
	std::unique_ptr<Zenyth::ConstantBuffer<Zenyth::CameraData>> m_cameraConstantBuffer{};


	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent {};
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue {};


	void GetHardwareAdapter(
		IDXGIFactory1* pFactory,
		IDXGIAdapter1** ppAdapter,
		bool requestHighPerformanceAdapter = false);


	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();

	std::wstring GetAssetFullPath(const std::wstring& assetName);
	std::unique_ptr<Zenyth::ConstantBuffer<Zenyth::CameraData>> cbCamera = nullptr;

	Zenyth::Camera m_camera;
	// Rendering loop timer.
	DX::StepTimer                           m_timer;

	// Input devices.
	std::unique_ptr<DirectX::Keyboard>      m_keyboard;
	std::unique_ptr<DirectX::Mouse>         m_mouse;

};