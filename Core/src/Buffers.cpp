#include "pch.hpp"

#include "Core.hpp"
#include "Buffers.hpp"

namespace Zenyth {

	void Buffer::Create(ID3D12Device *device, const std::wstring &name, const uint32_t numElements, const uint32_t elementSize, const void *initialData)
	{
		m_pDevice = device;
		m_elementSize = elementSize;
		m_elementCount = numElements;
		m_bufferSize = elementSize * numElements;

		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		const auto ibResDesc = CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize);
		ThrowIfFailed(m_pDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&ibResDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_buffer.ReleaseAndGetAddressOf()))); // ensure buffer suppression

		SUCCEEDED(m_buffer->SetName(name.c_str()));

		if (initialData != nullptr) {
			UINT8* pDataBegin;

			Map(&pDataBegin);
			memcpy(pDataBegin, initialData, m_bufferSize);
			Unmap();
		}
	}

	D3D12_GPU_VIRTUAL_ADDRESS Buffer::GetGPUVirtualAddress() const {
		return m_buffer->GetGPUVirtualAddress();
	}

	void Buffer::Map(UINT8** pDataBegin) {
		assert(!m_mapped);
		const CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(m_buffer->Map(0, &readRange, reinterpret_cast<void**>(pDataBegin)));
		m_mapped = true;
	}

	void Buffer::Unmap() {
		assert(m_mapped);
		m_buffer->Unmap(0, nullptr);
		m_mapped = false;
	}
}