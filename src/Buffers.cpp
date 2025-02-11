#include "Buffers.hpp"

namespace Zenyth {
	Buffer::Buffer(ID3D12Device *device, size_t size)
		: m_pDevice(device)
	{
		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		const auto ibResDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
		ThrowIfFailed(m_pDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&ibResDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_buffer)));
	}

	D3D12_GPU_VIRTUAL_ADDRESS Buffer::GetGPUVirtualAddress() const {
		return m_buffer->GetGPUVirtualAddress();
	}

	void Buffer::Map(UINT8** pDataBegin) const {
		const CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(m_buffer->Map(0, &readRange, reinterpret_cast<void**>(pDataBegin)));
	}

	void Buffer::Unmap() const {
		m_buffer->Unmap(0, nullptr);
	}
}