#include "pch.hpp"

#include "Renderer/DepthStencilBuffer.hpp"

namespace Zenyth
{

	DepthStencilBuffer::~DepthStencilBuffer()
	{
		if (!m_dsvHandle.IsNull())
			m_dsvHeap->Free(m_dsvHandle);

		if (!m_srvHandle.IsNull())
			m_resourceHeap->Free(m_srvHandle);
	}

	void DepthStencilBuffer::Create(ID3D12Device *device, const std::wstring &name, const uint32_t width, const uint32_t height)
	{
		m_pDevice = device;

		m_format = DXGI_FORMAT_R32_TYPELESS;
		m_resourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 0.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		const auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_format, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			m_resourceState,
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

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		if (m_srvHandle.IsNull())
			m_srvHandle = m_resourceHeap->Alloc();

		m_pDevice->CreateShaderResourceView(m_buffer.Get(), &srvDesc, m_srvHandle.CPU());
	}
}