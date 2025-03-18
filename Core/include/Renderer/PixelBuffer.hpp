#pragma once

#include <DirectXColors.h>

#include "Renderer/Buffers.hpp"

namespace Zenyth
{
	class PixelBuffer : public GpuBuffer
	{
	public:
		explicit PixelBuffer(DescriptorHeap& rtvHeap, DescriptorHeap& resourceHeap) : m_resourceHeap(&resourceHeap), m_rtvHeap(&rtvHeap) {};

		DELETE_COPY_CTOR(PixelBuffer)
		DEFAULT_MOVE_CTOR(PixelBuffer)

		~PixelBuffer() override;

		void CreateFromSwapChain(const std::wstring& name, ID3D12Resource* swapChainBuffer);
		void Create(const std::wstring& name, uint32_t width, uint32_t height,
					DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::XMVECTORF32 clearColor = DirectX::Colors::Black,
					D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

		void Resize(uint32_t width, uint32_t height);

		[[nodiscard]] const DescriptorHandle& GetRTV() const { return m_rtvHandle; }
		[[nodiscard]] const DescriptorHandle& GetSRV() const { return m_srvHandle; }

		[[nodiscard]] DXGI_FORMAT GetFormat() const { return m_format; }
		[[nodiscard]] DirectX::XMVECTORF32 GetClearColor() const { return m_clearColor; }

		void CreateViews() override;

	protected:
		DescriptorHeap* m_resourceHeap;
		DescriptorHandle m_srvHandle {};

		DXGI_FORMAT m_format = DXGI_FORMAT_R8G8B8A8_UNORM;
		D3D12_RESOURCE_FLAGS m_flags = D3D12_RESOURCE_FLAG_NONE;

		std::wstring m_name;

	private:
		DescriptorHeap* m_rtvHeap;
		DescriptorHandle m_rtvHandle {};

		DirectX::XMVECTORF32 m_clearColor = DirectX::Colors::Black;
	};
}