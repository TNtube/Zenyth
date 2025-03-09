#include "pch.hpp"

#include "Renderer/CommandBatch.hpp"

#include "Renderer/Renderer.hpp"

namespace Zenyth {
	CommandBatch::CommandBatch(const D3D12_COMMAND_LIST_TYPE type) : m_listType(type)
	{
	}

	CommandBatch::~CommandBatch()
	{
	}

	CommandBatch CommandBatch::Begin(const D3D12_COMMAND_LIST_TYPE type)
	{
		CommandBatch guard(type);

		Renderer::pCommandManager->GetNewCommandList(type, &guard.m_commandList, &guard.m_commandAllocator);

		return guard;
	}

	uint64_t CommandBatch::End()
	{
		CommandQueue& queue = Renderer::pCommandManager->GetQueue(m_listType);

		const auto fence = queue.ExecuteCommandList(m_commandList.Get());

		queue.FreeAllocator(fence, m_commandAllocator);

		m_commandList.Reset();
		m_commandList = nullptr;

		return fence;
	}

	void CommandBatch::TransitionResource(GpuBuffer& resource, const D3D12_RESOURCE_STATES after, const bool sendBarrier)
	{
		const auto before = resource.m_resourceState;
		if (before != after)
		{
			D3D12_RESOURCE_BARRIER& barrier = m_resourceBarriers[m_barrierIndex++];
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = resource.m_buffer.Get();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = before;
			barrier.Transition.StateAfter = after;
		}

		resource.m_resourceState = after;

		if (sendBarrier || m_barrierIndex == 16)
		{
			SendResourceBarriers();
		}
	}

	void CommandBatch::CopyBuffer(GpuBuffer &dst, GpuBuffer &src)
	{
		TransitionResource(dst, D3D12_RESOURCE_STATE_COPY_DEST);
		TransitionResource(src, D3D12_RESOURCE_STATE_COPY_SOURCE, true);

		m_commandList->CopyResource(dst.m_buffer.Get(), src.m_buffer.Get());
	}

	void CommandBatch::CopyBufferRegion(GpuBuffer &dst, uint64_t dstOffset, GpuBuffer &src, uint64_t srcOffset, uint64_t numBytes)
	{
		TransitionResource(dst, D3D12_RESOURCE_STATE_COPY_DEST);
		TransitionResource(src, D3D12_RESOURCE_STATE_COPY_SOURCE, true);

		m_commandList->CopyBufferRegion(dst.m_buffer.Get(), dstOffset, src.m_buffer.Get(), srcOffset, numBytes);
	}

	void CommandBatch::SendResourceBarriers()
	{
		if (m_barrierIndex > 0)
			m_commandList->ResourceBarrier(m_barrierIndex, m_resourceBarriers);

		m_barrierIndex = 0;

	}
}
