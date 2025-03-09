#pragma once
#include "CommandAllocatorPool.hpp"
#include "Core.hpp"

namespace Zenyth
{
	class CommandQueue
	{
	public:
		explicit CommandQueue(const D3D12_COMMAND_LIST_TYPE type) : m_type(type), m_allocatorPool(type) {};

		DELETE_COPY_CTOR(CommandQueue)
		DEFAULT_MOVE_CTOR(CommandQueue)

		~CommandQueue() { Destroy(); };

		void Create();
		void Destroy();

		uint64_t ExecuteCommandList(ID3D12GraphicsCommandList *commandList);

		bool IsFenceComplete(uint64_t fenceValue);
		void WaitForFence(uint64_t fenceValue);
		void WaitForIdle();

		ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.Get(); }

		ID3D12CommandAllocator* AcquireAllocator();
		void FreeAllocator(uint64_t fenceValue, ID3D12CommandAllocator *commandAllocator);

	private:
		D3D12_COMMAND_LIST_TYPE m_type;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		HANDLE m_fenceEvent {};
		uint64_t m_currentFenceValue = 0;
		uint64_t m_lastCompletedFenceValue = 0;
		std::mutex m_mutex;

		CommandAllocatorPool m_allocatorPool;
	};
}
