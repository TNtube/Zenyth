#include "pch.hpp"

#include "Core.hpp"
#include "Renderer/Buffers.hpp"

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

		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD); // todo: change to default
		const auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize);
		ThrowIfFailed(m_pDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_buffer.ReleaseAndGetAddressOf()))); // ensure buffer suppression

		SUCCEEDED(m_buffer->SetName(name.c_str()));

		if (initialData != nullptr) {
			UINT8* pDataBegin;

			Map(&pDataBegin);
			memcpy(pDataBegin, initialData, m_bufferSize);
			Unmap();
			// CommandContext::InitializeBuffer(m_buffer.Get(), initialData, m_bufferSize);
		}

		CreateViews();
	}

	void GpuBuffer::Map(UINT8** pDataBegin) {
		assert(!m_mapped);
		const CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(m_buffer->Map(0, &readRange, reinterpret_cast<void**>(pDataBegin)));
		m_mapped = true;
	}

	void GpuBuffer::Unmap() {
		assert(m_mapped);
		m_buffer->Unmap(0, nullptr);
		m_mapped = false;
	}


	void VertexBuffer::CreateViews()
	{
		m_vbv.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_vbv.StrideInBytes = m_elementSize;
		m_vbv.SizeInBytes = m_bufferSize;
	}


	void ColorBuffer::Create(const std::wstring &name, uint32_t width, uint32_t height)
	{
		m_pDevice = Renderer::pDevice.Get();

		m_elementCount = width * height;
		m_elementSize = sizeof(uint32_t) * 4;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		clearValue.Color[0] = 0.0f;
		clearValue.Color[1] = 0.0f;
		clearValue.Color[2] = 0.0f;
		clearValue.Color[3] = 1.0f;

		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		const auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1);
		ThrowIfFailed(m_pDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			&clearValue,
			IID_PPV_ARGS(m_buffer.ReleaseAndGetAddressOf())));

		SUCCEEDED(m_buffer->SetName(name.c_str()));

		CreateViews();
	}

	void ColorBuffer::CreateFromSwapChain(const std::wstring &name, ID3D12Resource* swapChainBuffer)
	{
		m_buffer.Reset();
		m_pDevice = Renderer::pDevice.Get();

		m_buffer.Attach(swapChainBuffer);
		SUCCEEDED(m_buffer->SetName(name.c_str()));

		if (m_rtvHandle.IsNull())
			m_rtvHandle = m_rtvHeap->Alloc();

		m_pDevice->CreateRenderTargetView(m_buffer.Get(), nullptr, m_rtvHandle.CPU());

		m_resourceState = D3D12_RESOURCE_STATE_PRESENT;
	}

	void ColorBuffer::CreateViews()
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		if (m_rtvHandle.IsNull())
			m_rtvHandle = m_resourceHeap->Alloc();

		m_pDevice->CreateRenderTargetView(m_buffer.Get(), &rtvDesc, m_rtvHandle.CPU());

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		if (m_srvHandle.IsNull())
			m_srvHandle = m_resourceHeap->Alloc();

		m_pDevice->CreateShaderResourceView(m_buffer.Get(), &srvDesc, m_srvHandle.CPU());
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

		Map(&m_mappedData);
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
