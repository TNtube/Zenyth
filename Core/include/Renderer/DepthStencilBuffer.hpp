#pragma once

#include "Renderer/PixelBuffer.hpp"

namespace Zenyth
{

	class DepthStencilBuffer final : public PixelBuffer
	{
	public:
		explicit DepthStencilBuffer(DescriptorHeap& dsvHeap, DescriptorHeap& resourceHeap)
			: PixelBuffer(dsvHeap, resourceHeap), m_dsvHeap(&dsvHeap) {}

		DELETE_COPY_CTOR(DepthStencilBuffer)
		DEFAULT_MOVE_CTOR(DepthStencilBuffer)

		~DepthStencilBuffer() override;

		// no need to re-instantiate the buffer for resizing it
		void Create(ID3D12Device *device, const std::wstring& name, uint32_t width, uint32_t height);

		[[nodiscard]] const DescriptorHandle& GetDSV() const { return m_dsvHandle; }
	private:
		using PixelBuffer::Create;
		using PixelBuffer::CreateFromSwapChain;
		using PixelBuffer::GetRTV;

		void CreateViews() override;

		DescriptorHeap*  m_dsvHeap   {};
		DescriptorHandle m_dsvHandle {};
	};
}