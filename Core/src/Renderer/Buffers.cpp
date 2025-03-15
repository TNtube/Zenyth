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

	void GpuBuffer::Create(ID3D12Device *device, const std::wstring &name, const uint32_t numElements, const uint32_t elementSize, const void *initialData)
	{
		Destroy();

		m_pDevice = device;
		m_elementSize = elementSize;
		m_elementCount = numElements;
		m_bufferSize = elementSize * numElements;

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
			CommandBatch::InitializeBuffer(*this, initialData, m_bufferSize);

		CreateViews();
	}


	void UploadBuffer::Create(const std::wstring &name, const size_t size)
	{
		Destroy();

		m_pDevice = Renderer::pDevice.Get();

		m_bufferSize = size;

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


	DepthStencilBuffer::~DepthStencilBuffer()
	{
		if (m_dsvHandle.IsNull())
			return;

		m_dsvHeap->Free(m_dsvHandle);
	}

	void DepthStencilBuffer::Create(ID3D12Device *device, const std::wstring &name, const uint32_t width, const uint32_t height)
	{
		m_pDevice = device;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		const auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(m_buffer.ReleaseAndGetAddressOf())));

		SUCCEEDED(m_buffer->SetName(name.c_str()));

		CreateViews();
	}

	void DepthStencilBuffer::CreateViews()
	{

		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		if (m_dsvHandle.IsNull()) {
			m_dsvHandle = m_dsvHeap->Alloc();
		}

		m_pDevice->CreateDepthStencilView(m_buffer.Get(), &depthStencilDesc, m_dsvHandle.CPU());
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
