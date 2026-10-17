#pragma once
#include "IRenderer.hpp"
#include "directx/d3dx12.h"
#include <dxgi1_6.h>
#include "math/vector.hpp"
#include "Texture.hpp"
#include "math/matrix.hpp"

namespace zenyth::renderer {
	struct Vertex
	{
		math::vec3 position;
		math::vec2 uv;
	};

	struct Transform
	{
		math::vec3 position;
		math::vec3 rotation;
		math::vec3 scale = math::vec3::one();

		[[nodiscard]] math::mat4 getMatrix() const {
			return
				math::mat4::from_translation(position)
				* math::mat4::from_euler(rotation)
				* math::mat4::from_scale(scale);
		}
	};

	struct SceneConstantBuffer
	{
		math::mat4 transform;
		float padding[48]; // Padding so the constant buffer is 256-byte aligned.
	};

	struct FrameResources
	{
		std::uint64_t FenceValue;
		Microsoft::WRL::ComPtr<ID3D12Resource> RenderTarget;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;
		Microsoft::WRL::ComPtr<ID3D12Resource> ConstantBufferA;
		Microsoft::WRL::ComPtr<ID3D12Resource> ConstantBufferB;
		std::uint8_t* pCbvDataBeginA {};
		std::uint8_t* pCbvDataBeginB {};
	};


	class D3D12Renderer : public IRenderer {
	public:
		void Init(HWND hwnd, uint32_t width, uint32_t height) override;
		void BeginFrame() override;
		void EndFrame() override;
		void Resize(uint32_t width, uint32_t height) override;
	private:
		static const UINT FrameCount = 2;
		static const int ResourceCount = 3;
		// Pipeline objects.
		CD3DX12_VIEWPORT m_viewport{};
		CD3DX12_RECT m_scissorRect{};
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		std::uint32_t m_rtvDescriptorSize {};
		std::uint32_t m_cbvSrvDescriptorSize {};
		FrameResources m_frameResources[FrameCount] {};
		Transform m_transformA;
		Transform m_transformB;
		SceneConstantBuffer m_constantBufferDataA {};
		SceneConstantBuffer m_constantBufferDataB {};

		// App resources.
		Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencil;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
		Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;

		// Synchronization objects.
		std::uint32_t m_frameIndex {};
		HANDLE m_fenceEvent {};
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		std::uint64_t m_fenceValue {};

		HWND m_hwnd {};

		std::uint32_t m_width {};
		std::uint32_t m_height {};

		void LoadPipeline();
		void LoadAssets();

		static Texture GenerateTextureData();
		void PopulateCommandList() const;
		void WaitForFence(std::uint64_t fenceValue) const;

		static void GetHardwareAdapter(IDXGIFactory1* pFactory,IDXGIAdapter1** ppAdapter,bool requestHighPerformanceAdapter = false);
	};
}
