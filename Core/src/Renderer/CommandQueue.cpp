#include "pch.hpp"

#include "Renderer/CommandQueue.hpp"

#include "Renderer/Renderer.hpp"

namespace Zenyth
{
	void CommandQueue::Create(ID3D12Device* device)
	{
		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};

		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = m_type;

		ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)), "Failed to create graphic command queue");
		SUCCEEDED(m_commandQueue->SetName(std::format(L"Command Queue {}", static_cast<int>(m_type)).c_str()));

		SUCCEEDED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		SUCCEEDED(m_fence->SetName(std::format(L"Command Queue Fence {}", static_cast<int>(m_type)).c_str()));

		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}

	void CommandQueue::Destroy()
	{
		if (m_commandQueue == nullptr)
			return;

		m_allocatorPool.Clear();

		CloseHandle(m_fenceEvent);

		m_fence.Reset();

		m_commandQueue.Reset();
	}

	uint64_t CommandQueue::ExecuteCommandList(ID3D12GraphicsCommandList *commandList)
	{
		std::lock_guard lock(m_mutex);

		SUCCEEDED(commandList->Close());

		ID3D12CommandList* commandLists[] = { commandList };
		m_commandQueue->ExecuteCommandLists(1, commandLists);

		SUCCEEDED(m_commandQueue->Signal(m_fence.Get(), m_currentFenceValue));

		return m_currentFenceValue++;
	}

	bool CommandQueue::IsFenceComplete(const uint64_t fenceValue)
	{
		if (fenceValue > m_lastCompletedFenceValue)
		{
			m_lastCompletedFenceValue = std::max(m_lastCompletedFenceValue, m_fence->GetCompletedValue());
		}

		return fenceValue <= m_lastCompletedFenceValue;
	}

	void CommandQueue::WaitForFence(const uint64_t fenceValue)
	{
		if (IsFenceComplete(fenceValue))
			return;

		std::unique_lock lock(m_mutex);

		SUCCEEDED(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
		m_lastCompletedFenceValue = fenceValue;
	}

	void CommandQueue::WaitForIdle()
	{
		uint64_t fence = 0;
		{
			std::lock_guard lock(m_mutex);
			SUCCEEDED(m_commandQueue->Signal(m_fence.Get(), m_currentFenceValue));
			fence = m_currentFenceValue++;
		}
		WaitForFence(fence);
	}

	ID3D12CommandAllocator* CommandQueue::AcquireAllocator()
	{
		const auto fence = m_fence->GetCompletedValue();
		return m_allocatorPool.AcquireAllocator(fence);
	}

	void CommandQueue::FreeAllocator(const uint64_t fenceValue, ID3D12CommandAllocator *commandAllocator)
	{
		m_allocatorPool.FreeAllocator(fenceValue, commandAllocator);
	}

	BufferView CommandQueue::AllocateUploadBufferView(const size_t size, const size_t alignment)
	{
		return m_uploadAllocatorPool.Allocate(size, alignment);
	}

	void CommandQueue::FreeUploadBufferView(const uint64_t fenceValue, const BufferView& buffer) const
	{
		m_uploadAllocatorPool.Return(fenceValue, buffer);
	}
}
