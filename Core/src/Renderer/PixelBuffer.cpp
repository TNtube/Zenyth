#include "pch.hpp"

#include "Renderer/PixelBuffer.hpp"

#include "Application.hpp"
#include "Renderer/Renderer.hpp"

namespace Zenyth
{

	void PixelBuffer::Create(const std::wstring& name, const uint32_t width, const uint32_t height,
							 const DXGI_FORMAT format, const DirectX::XMVECTORF32 clearColor, D3D12_RESOURCE_FLAGS flags)
	{
		auto& renderer = Application::Get().GetRenderer();
		m_pDevice = renderer.GetDevice();

		flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		m_flags = flags;

		m_resourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;

		m_format = format;
		m_clearColor = clearColor;

		m_name = name;

		Resize(width, height);
	}

	void PixelBuffer::Resize(const uint32_t width, const uint32_t height)
	{
		if (m_buffer)
			Destroy();

		m_elementCount = width * height;
		m_elementSize = sizeof(uint32_t) * 4;
		m_bufferSize = m_elementCount * m_elementSize;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = m_format;
		clearValue.Color[0] = m_clearColor.f[0];
		clearValue.Color[1] = m_clearColor.f[1];
		clearValue.Color[2] = m_clearColor.f[2];
		clearValue.Color[3] = m_clearColor.f[3];

		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		const auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_format, width, height, 1, 1, 1, 0, m_flags);
		ThrowIfFailed(m_pDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			m_resourceState,
			&clearValue,
			IID_PPV_ARGS(m_buffer.ReleaseAndGetAddressOf())));

		SUCCEEDED(m_buffer->SetName(m_name.c_str()));

		CreateViews();
	}

	PixelBuffer::~PixelBuffer()
	{
		if (!m_rtvHandle.IsNull())
			m_rtvHeap->Free(m_rtvHandle);

		if (!m_srvHandle.IsNull())
			m_resourceHeap->Free(m_srvHandle);

		if (m_buffer)
			m_buffer.Reset();
	}

	void PixelBuffer::CreateFromSwapChain(const std::wstring& name, ID3D12Resource* swapChainBuffer)
	{
		m_buffer.Reset();
		m_pDevice = Application::Get().GetRenderer().GetDevice();

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
		rtvDesc.Format = m_format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		if (m_rtvHandle.IsNull())
			m_rtvHandle = m_rtvHeap->Alloc();

		m_pDevice->CreateRenderTargetView(m_buffer.Get(), &rtvDesc, m_rtvHandle.CPU());

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = m_format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		if (m_srvHandle.IsNull())
			m_srvHandle = m_resourceHeap->Alloc();

		m_pDevice->CreateShaderResourceView(m_buffer.Get(), &srvDesc, m_srvHandle.CPU());
	}
}
