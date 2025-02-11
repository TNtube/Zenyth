#pragma once

#include "Window.hpp"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

constexpr uint32_t ScreenWidth = 1280;
constexpr uint32_t ScreenHeight = 720;

class Application
{
public:
	Application(HINSTANCE hInstance, bool useWrapDevice);
	void Run();

	void OnInit();
	void OnUpdate();
	void OnRender();
	void OnDestroy();

	void OnWindowSizeChanged(int width, int height);

	void OnKeyDown(uint8_t key);
	void OnKeyUp(uint8_t key);
private:
	std::unique_ptr<Window> m_window;

	static constexpr UINT FrameCount = 2;
	static constexpr UINT TextureWidth = 256;
	static constexpr UINT TextureHeight = 256;
	static constexpr UINT TexturePixelSize = 4;    // The number of bytes used to represent a pixel in the texture.

	bool m_useWarpDevice = false;

	float m_aspectRatio;


	struct SceneConstantBuffer
	{
		XMFLOAT4 offset;
		float padding[60]; // Padding so the constant buffer is 256-byte aligned.
	};

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
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView {};
	ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView {};
	ComPtr<ID3D12Resource> m_texture;
	ComPtr<ID3D12Resource> m_constantBuffer;
	SceneConstantBuffer m_constantBufferData {};
	UINT8* m_pCbvDataBegin;


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
	std::vector<UINT8> GenerateTextureData();
	void PopulateCommandList();
	void WaitForPreviousFrame();

	std::wstring GetAssetFullPath(const std::wstring& assetName);

};