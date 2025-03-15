#include "pch.hpp"

#include "Renderer/PixelBuffer.hpp"

#include "Renderer/Renderer.hpp"

namespace Zenyth
{

	void PixelBuffer::Create(const std::wstring &name, const uint32_t width, const uint32_t height)
	{
		m_pDevice = Renderer::pDevice.Get();

		m_elementCount = width * height;
		m_elementSize = sizeof(uint32_t) * 4;
		m_bufferSize = m_elementCount * m_elementSize;

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

	void PixelBuffer::CreateFromSwapChain(const std::wstring &name, ID3D12Resource* swapChainBuffer)
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

	void PixelBuffer::CreateViews()
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
}
