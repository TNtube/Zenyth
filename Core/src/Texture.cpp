#include "pch.hpp"

#include "Texture.hpp"
#include "Core.hpp"

namespace Zenyth
{

	Texture::Texture(ID3D12Device* device, DescriptorHeap& resourceHeap)
		:	m_pDevice(device), m_descriptorHandle(resourceHeap.Alloc())
	{

	}

	void Texture::Apply(ID3D12GraphicsCommandList *commandList, const uint32_t tableIndex) const
	{
		commandList->SetGraphicsRootDescriptorTable(tableIndex, m_descriptorHandle.GPU());
	}

	std::unique_ptr<Texture> Texture::LoadTextureFromFile(const wchar_t *filename, ID3D12Device* device, ID3D12Resource* uploadHeap, DescriptorHeap& resourceHeap, ID3D12GraphicsCommandList* commandList)
	{
		// Copy data to the intermediate upload heap and then schedule a copy
		// from the upload heap to the Texture2D.
		// std::vector<UINT8> texture = GenerateTextureData();

		auto output = std::make_unique<Texture>(device, resourceHeap);

		DirectX::TexMetadata info {};
		auto image = std::make_unique<DirectX::ScratchImage>();
		ThrowIfFailed(LoadFromDDSFile( filename,
		                               DirectX::DDS_FLAGS_NONE, &info, *image ));


		CD3DX12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(info.format, info.width, info.height, 1, info.mipLevels);

		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&output->m_texture)));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(output->m_texture.Get(), 0, 1);

		heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		textureDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		// Create the GPU upload buffer.
		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadHeap)));

		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = image->GetPixels();
		textureData.RowPitch = info.width * 4;
		textureData.SlicePitch = textureData.RowPitch * info.height;

		UpdateSubresources<1>(commandList, output->m_texture.Get(), uploadHeap, 0, 0, 1, &textureData);
		auto transition = CD3DX12_RESOURCE_BARRIER::Transition(output->m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(1, &transition);

		// Describe and create a SRV for the texture.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;

		device->CreateShaderResourceView(output->m_texture.Get(), &srvDesc, output->GetDescriptorHandle().CPU());

		auto msg = std::format("Texture buffer: {}", reinterpret_cast<const char *>(filename));
		const std::wstring wmsg(msg.begin(), msg.end());
		SUCCEEDED(output->m_texture->SetName(wmsg.c_str()));

		return std::move(output);
	}
}