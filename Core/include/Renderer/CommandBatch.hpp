#pragma once

#include "Buffers.hpp"
#include "Core.hpp"
#include "Material.hpp"
#include "Texture.hpp"

namespace Zenyth {
	struct BufferView;

	class CommandBatch
	{
	public:
		DELETE_COPY_CTOR(CommandBatch)
		DEFAULT_MOVE_CTOR(CommandBatch)

		~CommandBatch();

		static CommandBatch Begin(D3D12_COMMAND_LIST_TYPE type);
		uint64_t End(bool wait = true);

		static void InitializeBuffer(GpuBuffer& buffer, const void* data, size_t size, size_t offset = 0);
		static void InitializeTexture(Texture& texture, size_t subresourceCount, const D3D12_SUBRESOURCE_DATA *subData);

		void GenerateMips(Texture& texture);
		void GenerateMips(Texture& texture, bool isSRGB);

		void TransitionBarrier(GpuResource& resource, D3D12_RESOURCE_STATES after, bool sendBarrier = false);
		void AliasingBarrier(GpuResource& beforeResource, GpuResource& afterResource, bool sendBarriers = false);
		void UAVBarrier(GpuResource& resource, bool sendBarriers = false);

		template<typename T>
		void UploadToBuffer(GpuBuffer& buffer, const T& data, const uint32_t offset) { UploadToBuffer(buffer, &data, sizeof(data), offset); }
		void UploadToBuffer(GpuBuffer& buffer, const void* data, size_t size, uint32_t offset);

		void CopyBuffer(GpuResource& dst, GpuResource& src);
		void CopyBufferRegion(GpuBuffer& dst, uint64_t dstOffset, GpuBuffer& src, uint64_t srcOffset, uint64_t numBytes);

		void SubmitMaterial(std::shared_ptr<Material> material);
		void SetRootParameter(uint32_t idx, const DescriptorHandle& handle);

		void Dispatch(uint32_t x, uint32_t y, uint32_t z) const;

		[[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }

	private:
		explicit CommandBatch(D3D12_COMMAND_LIST_TYPE type);
		void SendResourceBarriers();

		D3D12_COMMAND_LIST_TYPE                           m_listType;
		ID3D12CommandAllocator*                           m_commandAllocator = nullptr;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList      = nullptr;


		static constexpr uint32_t                         ResourceBarrierCount = 16;

		D3D12_RESOURCE_BARRIER                            m_resourceBarriers[ResourceBarrierCount] = {};
		uint32_t                                          m_barrierIndex = 0;

		std::shared_ptr<Material>                         m_currentMaterial = nullptr;

		std::vector<std::pair<uint32_t, DescriptorHandle>>m_rootParameters {};

		std::vector<BufferView>                           m_usedBufferViews {};
	};

}
