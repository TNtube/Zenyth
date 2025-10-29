#pragma once
#include "CommandAllocatorPool.hpp"
#include "Core.hpp"
#include "UploadAllocator.hpp"

namespace Zenyth
{
	class CommandQueue
	{
	public:
		explicit CommandQueue(const D3D12_COMMAND_LIST_TYPE type)
			: m_type(type),
			  m_currentFenceValue(static_cast<uint64_t>(type) << 56 | 1),
			  m_lastCompletedFenceValue(static_cast<uint64_t>(type) << 56),
			  m_allocatorPool(type),
			  m_uploadAllocatorPool(type) {}

		DELETE_COPY_CTOR(CommandQueue)
		DELETE_MOVE_CTOR(CommandQueue)

		~CommandQueue() { Destroy(); };

		void Create(ID3D12Device* device);
		void Destroy();

		uint64_t ExecuteCommandList(ID3D12GraphicsCommandList *commandList);

		bool IsFenceComplete(uint64_t fenceValue);
		void WaitForFence(uint64_t fenceValue);
		void WaitForIdle();

		[[nodiscard]] ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.Get(); }

		ID3D12CommandAllocator* AcquireAllocator();
		void FreeAllocator(uint64_t fenceValue, ID3D12CommandAllocator *commandAllocator);

		BufferView AllocateUploadBufferView(size_t size, size_t alignment = 256);
		void FreeUploadBufferView(uint64_t fenceValue, const BufferView &buffer) const;

	private:
		D3D12_COMMAND_LIST_TYPE m_type;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		HANDLE m_fenceEvent {};
		uint64_t m_currentFenceValue = 0;
		uint64_t m_lastCompletedFenceValue = 0;
		std::mutex m_mutex;

		CommandAllocatorPool m_allocatorPool;
		UploadAllocatorPool m_uploadAllocatorPool;
	};
}
