#pragma once
#include "Buffers.hpp"
#include "DescriptorHeap.hpp"

namespace Zenyth
{
	using Microsoft::WRL::ComPtr;

	class Texture : public GpuBuffer
	{
	public:
		Texture(ID3D12Device *device, DescriptorHeap &resourceHeap);

		DELETE_COPY_CTOR(Texture)
		DEFAULT_MOVE_CTOR(Texture)

		const DescriptorHandle& GetDescriptorHandle() const { return m_descriptorHandle; }

		static std::unique_ptr<Texture> LoadTextureFromFile(const wchar_t *filename, DescriptorHeap &resourceHeap);

		void Apply(ID3D12GraphicsCommandList* commandList, uint32_t tableIndex) const;
	private:
		void CreateViews() override;
		DescriptorHeap* m_resourceHeap;
		DescriptorHandle m_descriptorHandle;
	};


}
