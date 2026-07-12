#pragma once
#include "IRenderer.hpp"
#include "directx/d3dx12.h"
#include <dxgi1_6.h>
#include "math/vector.hpp"
#include "Texture.hpp"

namespace zenyth::renderer {
	class D3D12Renderer : public IRenderer {
	public:
		void Init(HWND hwnd, uint32_t width, uint32_t height) override;
		void BeginFrame() override;
		void EndFrame() override;
		void Resize(uint32_t width, uint32_t height) override;
	private:
		static const UINT FrameCount = 2;

		struct Vertex
		{
			zenyth::math::vec3 position;
			zenyth::math::vec2 uv;
		};
		struct SceneConstantBuffer
		{
			zenyth::math::vec4 offset;
			float padding[60]; // Padding so the constant buffer is 256-byte aligned.
		};

		// Pipeline objects.
		CD3DX12_VIEWPORT m_viewport{};
		CD3DX12_RECT m_scissorRect{};
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
		std::uint32_t m_rtvDescriptorSize {};

		// App resources.
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
		Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;
		SceneConstantBuffer m_constantBufferData {};
		std::uint8_t* m_pCbvDataBegin {};

		// Synchronization objects.
		std::uint32_t m_frameIndex {};
		HANDLE m_fenceEvent {};
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValue {};

		HWND m_hwnd {};

		std::uint32_t m_width {};
		std::uint32_t m_height {};

		void LoadPipeline();
		void LoadAssets();

		static Texture GenerateTextureData();
		void PopulateCommandList() const;
		void WaitForPreviousFrame();

		static void GetHardwareAdapter(IDXGIFactory1* pFactory,IDXGIAdapter1** ppAdapter,bool requestHighPerformanceAdapter = false);
	};
}
