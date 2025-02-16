#pragma once

#include "Application.hpp"
#include "Buffers.hpp"
#include "Vertex.hpp"
#include "Texture.hpp"
#include "Camera.hpp"
#include "StepTimer.h"

using Microsoft::WRL::ComPtr;

class Minicraft final : public Zenyth::Application
{
public:
	Minicraft(uint32_t width, uint32_t height, bool useWrapDevice);

	void OnInit() override;
	void Tick() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnDestroy() override;

	void OnWindowSizeChanged(int width, int height) override;

private:
	float m_aspectRatio;


	struct SceneConstantBuffer
	{
		DirectX::SimpleMath::Matrix model;
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
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
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
	UINT64 m_fenceValues[FrameCount] {};


	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList() const;

	void WaitForGpu();
	void MoveToNextFrame();

	static std::wstring GetAssetFullPath(const std::wstring& assetName);
	std::unique_ptr<Zenyth::ConstantBuffer<Zenyth::CameraData>> cbCamera = nullptr;

	Zenyth::Camera m_camera;
	// Rendering loop timer.
	DX::StepTimer                           m_timer;

	// Input devices.
	std::unique_ptr<DirectX::Keyboard>      m_keyboard;
	std::unique_ptr<DirectX::Mouse>         m_mouse;
};
