#pragma once
#include "CommandQueue.hpp"

namespace Zenyth
{
	class CommandManager
	{
	public:
		CommandManager()
			: m_graphicsQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
			  m_copyQueue(D3D12_COMMAND_LIST_TYPE_COPY),
			  m_computeQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE) {}

		void Create(ID3D12Device* device);
		void Destroy();

		~CommandManager() { Destroy(); }

		CommandQueue& GetGraphicsQueue() { return m_graphicsQueue; }
		CommandQueue& GetCopyQueue() { return m_copyQueue; }
		CommandQueue& GetComputeQueue() { return m_computeQueue; }

		CommandQueue& GetQueue(const D3D12_COMMAND_LIST_TYPE type)
		{
			switch (type)
			{
			case D3D12_COMMAND_LIST_TYPE_DIRECT:
				return m_graphicsQueue;
			case D3D12_COMMAND_LIST_TYPE_COPY:
				return m_copyQueue;
			case D3D12_COMMAND_LIST_TYPE_COMPUTE:
				return m_computeQueue;
			default:
				assert(false);
				return m_graphicsQueue;
			}
		}

		bool IsFenceComplete(uint64_t fenceValue);
		void WaitForFence(uint64_t fenceValue);


		void GetNewCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList** list, ID3D12CommandAllocator** allocator);

		void IdleGPU()
		{
			m_graphicsQueue.WaitForIdle();
			m_copyQueue.WaitForIdle();
			m_computeQueue.WaitForIdle();
		}

	private:
		CommandQueue m_graphicsQueue;
		CommandQueue m_copyQueue;
		CommandQueue m_computeQueue;
	};
}
