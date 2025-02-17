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
			IID_PPV_ARGS(&m_buffer)));

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

	D3D12_VERTEX_BUFFER_VIEW Buffer::CreateVertexBufferView() const {
		D3D12_VERTEX_BUFFER_VIEW vbv;

		vbv.BufferLocation = m_buffer->GetGPUVirtualAddress();
		vbv.StrideInBytes = m_elementSize;
		vbv.SizeInBytes = m_bufferSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW Buffer::CreateIndexBufferView() const {
		D3D12_INDEX_BUFFER_VIEW ibv;

		ibv.BufferLocation = m_buffer->GetGPUVirtualAddress();
		ibv.Format = DXGI_FORMAT_R32_UINT;
		ibv.SizeInBytes = m_bufferSize;

		return ibv;
	}

	DescriptorHandle Buffer::CreateDescriptor(DescriptorHeap &descriptorHeap) const {
		// Describe and create a constant buffer view.
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_buffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = m_bufferSize;

		const auto cbvHandle = descriptorHeap.Alloc(1);
		m_pDevice->CreateConstantBufferView(&cbvDesc, cbvHandle.CPU());

		return cbvHandle;
	}

	void Buffer::Map(UINT8** pDataBegin) const {
		const CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(m_buffer->Map(0, &readRange, reinterpret_cast<void**>(pDataBegin)));
	}

	void Buffer::Unmap() const {
		m_buffer->Unmap(0, nullptr);
	}
}