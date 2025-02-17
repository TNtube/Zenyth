#pragma once
#include "DescriptorHeap.hpp"

namespace Zenyth {
	using Microsoft::WRL::ComPtr;

	class Buffer
	{
	public:
		void Create(ID3D12Device *device, const std::wstring& name, uint32_t numElements, uint32_t elementSize, const void* initialData = nullptr);

		[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

		[[nodiscard]] D3D12_VERTEX_BUFFER_VIEW CreateVertexBufferView() const;
		[[nodiscard]] D3D12_INDEX_BUFFER_VIEW CreateIndexBufferView() const;
		DescriptorHandle CreateDescriptor(DescriptorHeap& descriptorHeap) const;

		void Map(UINT8** pDataBegin) const;
		void Unmap() const;

		uint32_t GetElementCount() const { return m_elementCount; };

	protected:
		ComPtr<ID3D12Resource>	m_buffer {};
		ID3D12Device*			m_pDevice {};
		size_t					m_bufferSize {};
		uint32_t				m_elementCount {};
		uint32_t				m_elementSize {};
	};
}
