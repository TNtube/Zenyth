#include "pch.hpp"

#include "Renderer/Texture.hpp"
#include "Core.hpp"
#include "DDSTextureLoader.h"
#include "DirectXHelpers.h"
#include "ResourceUploadBatch.h"
#include "Renderer/CommandBatch.hpp"
#include "Renderer/Renderer.hpp"

namespace Zenyth
{

	Texture::Texture(ID3D12Device* device, DescriptorHeap& resourceHeap)
		:	m_resourceHeap(&resourceHeap)
	{
		m_pDevice = device;
		m_resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
	}

	void Texture::CreateViews()
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = m_buffer->GetDesc().Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = m_buffer->GetDesc().MipLevels;

		if (m_srvHandle.IsNull())
			m_srvHandle = m_resourceHeap->Alloc();

		m_pDevice->CreateShaderResourceView(m_buffer.Get(), &srvDesc, m_srvHandle.CPU());
	}

	std::unique_ptr<Texture> Texture::LoadTextureFromFile(const wchar_t *filename, DescriptorHeap& resourceHeap)
	{
		auto* device = Renderer::pDevice.Get();
		auto output = std::make_unique<Texture>(device, resourceHeap);

		std::unique_ptr<uint8_t[]> ddsData;
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;

		ThrowIfFailed(
			DirectX::LoadDDSTextureFromFile(device, filename, output->m_buffer.ReleaseAndGetAddressOf(),
				ddsData, subresources));

		CommandBatch::InitializeTexture(*output, static_cast<uint32_t>(subresources.size()), subresources.data());

		output->CreateViews();

		return output;
	}
}
