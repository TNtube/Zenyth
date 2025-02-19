#pragma once

#include "Application.hpp"
#include "Buffers.hpp"
#include "Vertex.hpp"
#include "Texture.hpp"
#include "Camera.hpp"
#include "World.hpp"
#include "DescriptorHeap.hpp"
#include "StepTimer.hpp"

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

	Microsoft::WRL::ComPtr<ID3D12Debug> m_debug;
	Microsoft::WRL::ComPtr<ID3D12InfoQueue1> m_infoQueue;

	DWORD m_callbackCookie{};

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	std::unique_ptr<Zenyth::DescriptorHeap> m_rtvHeap {};
	std::unique_ptr<Zenyth::DescriptorHeap> m_dsvHeap {};
	std::unique_ptr<Zenyth::DescriptorHeap> m_resourceHeap {};
	Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	std::unique_ptr<Zenyth::DepthStencilBuffer> m_depthStencilBuffers[FrameCount];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;


	// App resources.
	std::unique_ptr<Zenyth::Texture> m_texture {};
	std::unique_ptr<Zenyth::ConstantBuffer<Zenyth::CameraData>> m_cameraConstantBuffer {};

	std::unique_ptr<World> m_world {};

	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent {};
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[FrameCount] {};


	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList() const;

	void WaitForGpu();
	void MoveToNextFrame();

	static std::wstring GetAssetFullPath(const std::wstring& assetName);

	Zenyth::Camera m_camera;
	// Rendering loop timer.
	DX::StepTimer                           m_timer;

	// Input devices.
	std::unique_ptr<DirectX::Keyboard>      m_keyboard;
	std::unique_ptr<DirectX::Mouse>         m_mouse;
};
