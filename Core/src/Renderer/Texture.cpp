#include "pch.hpp"

#include "Renderer/Texture.hpp"
#include "DDSTextureLoader.h"
#include "DirectXHelpers.h"
#include "ResourceUploadBatch.h"
#include "Renderer/CommandBatch.hpp"
#include "Renderer/Renderer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Zenyth
{

	Texture::Texture(ID3D12Device* device, DescriptorHeap& resourceHeap)
		:	m_resourceHeap(&resourceHeap)
	{
		m_pDevice = device;
		m_resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
	}

	Texture::~Texture()
	{
		if (m_resourceHeap && !m_srvHandle.IsNull())
		{
			m_resourceHeap->Free(m_srvHandle);
		}
	}

	void Texture::CreateViews()
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = m_buffer->GetDesc().Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1; //m_buffer->GetDesc().MipLevels;

		if (m_srvHandle.IsNull())
			m_srvHandle = m_resourceHeap->Alloc();

		m_pDevice->CreateShaderResourceView(m_buffer.Get(), &srvDesc, m_srvHandle.CPU());
	}

	std::unique_ptr<Texture> Texture::LoadTextureFromFile(const char *filename, DescriptorHeap& resourceHeap, const bool sRGB)
	{
		auto* device = Renderer::pDevice.Get();
		auto output = std::make_unique<Texture>(device, resourceHeap);

		int image_width = 0;
		int image_height = 0;
		int channel_count = 0;

		unsigned char* data = stbi_load(filename, &image_width, &image_height, &channel_count, 0);
		if (data == nullptr)
			return nullptr;

		// Create texture resource
		D3D12_HEAP_PROPERTIES props = {};
		props.Type = D3D12_HEAP_TYPE_DEFAULT;
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Alignment = 0;
		desc.Width = image_width;
		desc.Height = image_height;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = channel_count == 1
					  ? DXGI_FORMAT_R8_UNORM
					  : sRGB
					    ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
					    : DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		Renderer::pDevice->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
			   D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(output->m_buffer.ReleaseAndGetAddressOf()));

		D3D12_SUBRESOURCE_DATA subresource;
		subresource.pData = data;
		subresource.RowPitch = image_width * channel_count;
		subresource.SlicePitch = subresource.RowPitch * image_height;

		CommandBatch::InitializeTexture(*output, 1u, &subresource);

		output->CreateViews();

		stbi_image_free(data);

		return output;
	}
}
