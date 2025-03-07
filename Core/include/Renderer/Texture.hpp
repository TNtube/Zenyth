#pragma once
#include "DescriptorHeap.hpp"

namespace Zenyth
{
	using Microsoft::WRL::ComPtr;

	class Texture
	{
	public:
		explicit Texture(ID3D12Device* device, DescriptorHeap& resourceHeap);

		~Texture() = default;

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		Texture(Texture&&) = default;
		Texture& operator=(Texture&&) = default;

		const DescriptorHandle& GetDescriptorHandle() const { return m_descriptorHandle; }

		static std::unique_ptr<Texture> LoadTextureFromFile(const wchar_t *filename, ID3D12Device *device, ID3D12CommandQueue *commandQueue, DescriptorHeap &resourceHeap, ID3D12GraphicsCommandList
		                                                    *commandList);
		void Apply(ID3D12GraphicsCommandList* commandList, uint32_t tableIndex) const;

	private:
		ID3D12Device* m_pDevice;
		DescriptorHandle m_descriptorHandle;
		ComPtr<ID3D12Resource> m_texture;
	};


}
