/*
 * Since command allocator need to be reseted before being reused, we need to keep track of
 * the command allocators that are "in-flight" and the ones that are "available" to be used.
 * This is done by using a CommandAllocatorPool.
 */


#pragma once

#include <mutex>
#include <queue>

#include "Core.hpp"


namespace Zenyth {
	class CommandAllocatorPool
	{
	public:
		explicit CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type);

		DELETE_COPY_CTOR(CommandAllocatorPool)
		DELETE_MOVE_CTOR(CommandAllocatorPool)

		~CommandAllocatorPool() { Clear(); }

		void Clear();

		ID3D12CommandAllocator* AcquireAllocator(uint64_t completedFenceValue);
		void FreeAllocator(uint64_t fenceValue, ID3D12CommandAllocator *commandAllocator);

		[[nodiscard]] size_t Size() const { return m_commandAllocators.size(); }


	private:
		struct AvailableAllocator
		{
			uint64_t fenceValue;
			ID3D12CommandAllocator* commandAllocator;
		};

		D3D12_COMMAND_LIST_TYPE m_type;
		std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_commandAllocators;
		std::queue<AvailableAllocator> m_availableCommandAllocators;
		std::mutex m_mutex;
	};
}