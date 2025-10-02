#include "pch.hpp"

#include "Core.hpp"
#include "Renderer/Buffers.hpp"

#include "Renderer/CommandBatch.hpp"
#include "Renderer/Renderer.hpp"

namespace Zenyth {
	GpuBuffer::~GpuBuffer()
	{
		Destroy();
	}

	void GpuBuffer::Create(ID3D12Device *device, const std::wstring& name, const size_t numElements, const size_t elementSize, const bool align, const void *initialData)
	{
		Destroy();

		m_pDevice = device;
		m_elementSize = align ? Math::AlignToMask(elementSize, 256ull) : elementSize;
		m_elementCount = numElements;
		m_bufferSize = m_elementSize * numElements;

		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		const auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize);
		ThrowIfFailed(m_pDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			m_resourceState,
			nullptr,
			IID_PPV_ARGS(m_buffer.ReleaseAndGetAddressOf()))); // ensure buffer suppression

		SUCCEEDED(m_buffer->SetName(name.c_str()));

		if (initialData != nullptr)
		{
			// we need to copy the initial data size, we cannot use the m_bufferSize
			// since it might actually be greater than the copied data.
			CommandBatch::InitializeBuffer(*this, initialData, elementSize * numElements);
		}

		CreateViews();
	}


	void UploadBuffer::Create(const std::wstring& name, const size_t size)
	{
		Destroy();

		m_pDevice = Renderer::pDevice.Get();

		m_bufferSize = Math::AlignToMask(size, 256ull);

		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		const auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize);

		ThrowIfFailed(m_pDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_buffer.ReleaseAndGetAddressOf())));

		SUCCEEDED(m_buffer->SetName(name.c_str()));
	}

	void* UploadBuffer::Map()
	{
		const CD3DX12_RANGE readRange(0, 0);
		SUCCEEDED(m_buffer->Map(0, &readRange, &m_memory));
		return m_memory;
	}

	void UploadBuffer::Unmap() const
	{
		m_buffer->Unmap(0, nullptr);
	}


	void VertexBuffer::CreateViews()
	{
		m_vbv.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_vbv.StrideInBytes = m_elementSize;
		m_vbv.SizeInBytes = m_bufferSize;
	}

	void IndexBuffer::CreateViews()
	{
		m_ibv.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_ibv.Format = DXGI_FORMAT_R32_UINT;
		m_ibv.SizeInBytes = m_bufferSize;
	}


	ConstantBuffer::~ConstantBuffer()
	{
		if (m_resourceHeap)
		{
			m_resourceHeap->Free(m_cbvHandle);
		}
	}

	void ConstantBuffer::CreateViews()
	{

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_buffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = m_bufferSize;

		if (m_cbvHandle.IsNull())
			m_cbvHandle = m_resourceHeap->Alloc();

		m_pDevice->CreateConstantBufferView(&cbvDesc, m_cbvHandle.CPU());
	}


	StructuredBuffer::~StructuredBuffer()
	{
		if (!m_srvHandle.IsNull())
			m_resourceHeap->Free(m_srvHandle);
	}

	void StructuredBuffer::CreateViews()
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = m_elementCount;
		srvDesc.Buffer.StructureByteStride = m_elementSize;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		m_srvHandle = m_resourceHeap->Alloc();
		m_pDevice->CreateShaderResourceView(m_buffer.Get(), &srvDesc, m_srvHandle.CPU());
	}
}
