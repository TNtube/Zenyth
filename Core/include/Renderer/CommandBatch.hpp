#pragma once

#include "Buffers.hpp"
#include "Core.hpp"
#include "Material.hpp"
#include "Texture.hpp"

namespace Zenyth {

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
		static void GenerateMips(Texture& texture);

		void TransitionBarrier(GpuResource* resource, D3D12_RESOURCE_STATES after, bool sendBarrier = false);
		void TransitionBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, bool sendBarrier = false);

		void AliasingBarrier(GpuResource* beforeResource, GpuResource* afterResource, bool sendBarriers = false);

		void CopyResource(GpuResource* dst, GpuResource* src);
		void CopyBufferRegion(GpuBuffer &dst, uint64_t dstOffset, GpuBuffer &src, uint64_t srcOffset, uint64_t numBytes);

		void SubmitMaterial(std::shared_ptr<Material> material);
		void SetRootParameter(uint32_t idx, const DescriptorHandle& handle);

		void Dispatch(uint32_t x, uint32_t y, uint32_t z) const;

		[[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }

	private:
		explicit CommandBatch(D3D12_COMMAND_LIST_TYPE type);

		void SendResourceBarriers();

		// we want to keep some objects alive for as long as the command list exists
		void TrackResource(const Microsoft::WRL::ComPtr<ID3D12Object>& object);
		void TrackResource(const GpuBuffer& buffer);

		D3D12_COMMAND_LIST_TYPE                           m_listType;
		ID3D12CommandAllocator*                           m_commandAllocator = nullptr;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList      = nullptr;


		static constexpr uint32_t                         ResourceBarrierCount = 16;

		D3D12_RESOURCE_BARRIER                            m_resourceBarriers[ResourceBarrierCount] = {};
		uint32_t                                          m_barrierIndex = 0;

		std::shared_ptr<Material>                         m_currentMaterial = nullptr;

		std::vector<std::pair<uint32_t, DescriptorHandle>> m_rootParameters {};


		std::vector<Microsoft::WRL::ComPtr<ID3D12Object>> m_trackedObjects;
	};

}
