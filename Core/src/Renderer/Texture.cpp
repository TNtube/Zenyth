#include "pch.hpp"

#include "Renderer/Texture.hpp"
#include "Core.hpp"
#include "DDSTextureLoader.h"
#include "DirectXHelpers.h"
#include "ResourceUploadBatch.h"

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

	std::unique_ptr<Texture> Texture::LoadTextureFromFile(const wchar_t *filename, ID3D12Device* device, ID3D12CommandQueue* commandQueue, DescriptorHeap& resourceHeap, ID3D12GraphicsCommandList* commandList)
	{
		auto output = std::make_unique<Texture>(device, resourceHeap);

		DirectX::ResourceUploadBatch resourceUpload(device);

		resourceUpload.Begin();

		ThrowIfFailed(
			CreateDDSTextureFromFile(device, resourceUpload, filename,
			output->m_texture.ReleaseAndGetAddressOf()));

		DirectX::CreateShaderResourceView(device, output->m_texture.Get(), output->m_descriptorHandle.CPU());

		auto uploadResourcesFinished = resourceUpload.End(commandQueue);

		uploadResourcesFinished.wait();

		const auto msg = std::format(L"Texture buffer: {}", std::wstring(filename));
		SUCCEEDED(output->m_texture->SetName(msg.c_str()));

		return output;
	}
}
