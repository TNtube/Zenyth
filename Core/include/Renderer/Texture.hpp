#pragma once
#include "Buffers.hpp"
#include "DescriptorHeap.hpp"

namespace Zenyth
{
	class Texture final : public GpuBuffer
	{
	public:
		Texture(ID3D12Device *device, DescriptorHeap &resourceHeap);

		DELETE_COPY_CTOR(Texture)
		DEFAULT_MOVE_CTOR(Texture)

		const DescriptorHandle& GetSRV() const { return m_srvHandle; }

		static std::unique_ptr<Texture> LoadTextureFromFile(const wchar_t *filename, DescriptorHeap &resourceHeap);
	private:
		void CreateViews() override;
		DescriptorHeap* m_resourceHeap;
		DescriptorHandle m_srvHandle;
	};


}
