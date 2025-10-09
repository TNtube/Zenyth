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
		static void InitializeTexture(Texture &texture, size_t subresourceCount, const D3D12_SUBRESOURCE_DATA *subData);

		void TransitionResource(GpuBuffer &resource, D3D12_RESOURCE_STATES after, bool sendBarrier = false);

		void CopyBuffer(GpuBuffer &dst, GpuBuffer &src);
		void CopyBufferRegion(GpuBuffer &dst, uint64_t dstOffset, GpuBuffer &src, uint64_t srcOffset, uint64_t numBytes);

		void SubmitMaterial(const Material* material);
		void SetRootParameter(const std::string& param, const DescriptorHandle& handle);

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

		const Material*                                   m_currentMaterial = nullptr;

		std::unordered_map<std::string, DescriptorHandle> m_rootParameters {};
	};

}
