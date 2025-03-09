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

	uint64_t CommandBatch::End(bool wait)
	{
		CommandQueue& queue = Renderer::pCommandManager->GetQueue(m_listType);

		const auto fence = queue.ExecuteCommandList(m_commandList.Get());

		queue.FreeAllocator(fence, m_commandAllocator);

		m_commandList.Reset();
		m_commandList = nullptr;

		if (wait)
			queue.WaitForFence(fence);

		return fence;
	}

	void CommandBatch::InitializeBuffer(GpuBuffer &buffer, const void *data, size_t size, size_t offset)
	{
		CommandBatch batch = Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);

		UploadBuffer uploadBuffer;
		uploadBuffer.Create(L"UploadBuffer", size);
		void* mappedData = uploadBuffer.Map();
		memcpy(mappedData, data, size);

		batch.TransitionResource(buffer, D3D12_RESOURCE_STATE_COPY_DEST, true);
		batch.CopyBufferRegion(buffer, offset, uploadBuffer, 0, size);
		batch.TransitionResource(buffer, D3D12_RESOURCE_STATE_GENERIC_READ, true);

		batch.End(true);
	}

	void CommandBatch::InitializeTexture(Texture& texture, const size_t subresourceCount, const D3D12_SUBRESOURCE_DATA* subData)
	{
		const auto uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, subresourceCount);

		CommandBatch batch = Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);

		UploadBuffer uploadBuffer;
		uploadBuffer.Create(L"UploadBuffer", uploadBufferSize);
		UpdateSubresources(batch.m_commandList.Get(), texture.Get(), uploadBuffer.Get(), 0, 0, subresourceCount, subData);
		batch.TransitionResource(texture, D3D12_RESOURCE_STATE_GENERIC_READ, true);

		batch.End(true);
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
