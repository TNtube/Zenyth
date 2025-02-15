#include "Buffers.hpp"

namespace Zenyth {
	Buffer::Buffer(ID3D12Device *device, size_t size)
		: m_pDevice(device), m_size(size)
	{
		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		const auto ibResDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size);
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

	IndexBuffer::IndexBuffer(ID3D12Device *device, const uint32_t* data, size_t size)
		: Buffer(device, size)
	{
		m_buffer->SetName(L"Index buffer");
		UINT8* pVertexDataBegin;

		Map(&pVertexDataBegin);
		memcpy(pVertexDataBegin, data, size);
		Unmap();

		m_bufferView.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_bufferView.Format = DXGI_FORMAT_R32_UINT;
		m_bufferView.SizeInBytes = size;
	}

	void IndexBuffer::Apply(ID3D12GraphicsCommandList *commandList) const
	{
		commandList->IASetIndexBuffer(&m_bufferView);
	}
}