#pragma once

#include "Renderer/Buffers.hpp"

namespace Zenyth
{
	class PixelBuffer final : public GpuBuffer
	{
	public:
		explicit PixelBuffer(DescriptorHeap& rtvHeap, DescriptorHeap& resourceHeap) : m_rtvHeap(&rtvHeap), m_resourceHeap(&resourceHeap) {};

		DELETE_COPY_CTOR(PixelBuffer)
		DEFAULT_MOVE_CTOR(PixelBuffer)

		~PixelBuffer() override = default;

		void CreateFromSwapChain(const std::wstring& name, ID3D12Resource* swapChainBuffer);
		void Create(const std::wstring& name, uint32_t width, uint32_t height,
					DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
					D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

		[[nodiscard]] const DescriptorHandle& GetRTV() const { return m_rtvHandle; }
		[[nodiscard]] const DescriptorHandle& GetSRV() const { return m_srvHandle; }
		void CreateViews() override;

	private:
		DescriptorHeap* m_rtvHeap;
		DescriptorHeap* m_resourceHeap;

		DescriptorHandle m_rtvHandle {};
		DescriptorHandle m_srvHandle {};
	};
}