#pragma once

#include "Application.hpp"
#include "Buffers.hpp"
#include "Vertex.hpp"
#include "Texture.hpp"
#include "Camera.hpp"
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

	DWORD m_callbackCookie{};

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Zenyth::DescriptorHeap m_rtvHeap {};
	Zenyth::DescriptorHeap m_resourceHeap {};
	Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;


	// App resources.
	std::unique_ptr<Zenyth::Buffer> m_vertexBuffer {};
	std::unique_ptr<Zenyth::Buffer> m_indexBuffer {};
	std::unique_ptr<Zenyth::Texture> m_texture {};
	Zenyth::Transform m_faceTransform {};

	std::unique_ptr<Zenyth::Buffer> m_constantBuffer1 {};
	uint8_t* m_constantBuffer1Begin {};
	Zenyth::DescriptorHandle m_cbv1Handle {};

	std::unique_ptr<Zenyth::Buffer> m_constantBuffer2 {};
	uint8_t* m_constantBuffer2Begin {};
	Zenyth::DescriptorHandle m_cbv2Handle {};

	std::unique_ptr<Zenyth::Buffer> m_cameraConstantBuffer {};
	uint8_t* m_cameraConstantBufferBegin {};
	Zenyth::DescriptorHandle m_cameraCbvHandle {};


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
