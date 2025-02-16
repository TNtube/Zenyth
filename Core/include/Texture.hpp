#pragma once

namespace Zenyth
{
	using Microsoft::WRL::ComPtr;

	class Texture
	{
	public:
		explicit Texture(ID3D12Device* device, ID3D12DescriptorHeap* resourceHeap, uint8_t offset);

		~Texture() = default;

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		Texture(Texture&&) = default;
		Texture& operator=(Texture&&) = default;


		static std::unique_ptr<Texture> LoadTextureFromFile(const wchar_t *filename, ID3D12Device* device, ID3D12Resource* uploadHeap, ID3D12DescriptorHeap* resourceHeap, ID3D12GraphicsCommandList* commandList, uint8_t offset = 0);
		void Apply(ID3D12GraphicsCommandList* commandList) const;

	private:
		ID3D12Device* m_pDevice;

		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle{};

		ComPtr<ID3D12Resource> m_texture;

		uint8_t m_offset;
	};


}