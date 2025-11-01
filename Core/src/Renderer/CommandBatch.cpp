#include "pch.hpp"

#include "Renderer/CommandBatch.hpp"

#include "Application.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/Renderer.hpp"

#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
	( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
	| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
	| D3D12_RESOURCE_STATE_COPY_DEST \
	| D3D12_RESOURCE_STATE_COPY_SOURCE )


CommandBatch::CommandBatch(const D3D12_COMMAND_LIST_TYPE type) : m_listType(type)
{
}

CommandBatch::~CommandBatch() = default;

CommandBatch CommandBatch::Begin(const D3D12_COMMAND_LIST_TYPE type)
{
	CommandBatch guard(type);

	auto& renderer = Application::Get().GetRenderer();
	renderer.GetCommandManager().GetNewCommandList(type, &guard.m_commandList, &guard.m_commandAllocator);

	return guard;
}

uint64_t CommandBatch::End(const bool wait)
{
	auto& renderer = Application::Get().GetRenderer();
	CommandQueue& queue = renderer.GetCommandManager().GetQueue(m_listType);

	SendResourceBarriers();

	const auto fence = queue.ExecuteCommandList(m_commandList.Get());

	queue.FreeAllocator(fence, m_commandAllocator);

	// we are finished with those upload buffers, we can free them
	for (const auto& bufferView : m_usedBufferViews)
		queue.FreeUploadBufferView(fence, bufferView);

	m_commandList.Reset();
	m_commandList = nullptr;

	if (wait)
		queue.WaitForFence(fence);

	return fence;
}

void CommandBatch::InitializeBuffer(GpuBuffer& buffer, const void *data, const size_t size, const size_t offset)
{
	CommandBatch batch = Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);
	batch.UploadToBuffer(buffer, data, size, offset);
	batch.End(true);
}

void CommandBatch::InitializeTexture(Texture& texture, const size_t subresourceCount, const D3D12_SUBRESOURCE_DATA* subData)
{
	const auto uploadBufferSize = GetRequiredIntermediateSize(texture.GetResource(), 0, subresourceCount);

	CommandBatch batch = Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);

	auto& renderer = Application::Get().GetRenderer();
	CommandQueue& queue = renderer.GetCommandManager().GetQueue(batch.m_listType);

	const BufferView bufferView = queue.AllocateUploadBufferView(uploadBufferSize);

	const auto beforeState = texture.GetState();

	UpdateSubresources(batch.m_commandList.Get(), texture.GetResource(), bufferView.buffer->GetResource(), bufferView.offset, 0, subresourceCount, subData);
	batch.TransitionBarrier(texture, beforeState, true);

	const auto fence = batch.End(true);

	queue.FreeUploadBufferView(fence, bufferView);
}

void CommandBatch::GenerateMips(Texture& texture)
{
	GenerateMips(texture, Texture::IsSRGBFormat( texture.GetResource()->GetDesc().Format ));
}

void CommandBatch::GenerateMips(Texture& texture, const bool isSRGB)
{
	struct alignas( 16 ) GenerateMipsCB
	{
		uint32_t SrcMipLevel;           // Texture level of source mip
		uint32_t NumMipLevels;          // Number of OutMips to write: [1-4]
		uint32_t SrcDimension;          // Width and height of the source texture are even or odd.
		uint32_t IsSRGB;                // Must apply gamma correction to sRGB textures.
		DirectX::XMFLOAT2 TexelSize;    // 1.0 / OutMip1.Dimensions
	};

	static std::unique_ptr<Pipeline> generateMipsPipeline;
	if (!generateMipsPipeline) // todo: better caching to prevent leak
	{
		generateMipsPipeline = std::make_unique<Pipeline>();
		generateMipsPipeline->Create(L"Generate Mips PSO", L"resources/shaders/GenerateMipsCS.hlsl");
	}

	m_commandList->SetPipelineState(generateMipsPipeline->Get());
	m_commandList->SetComputeRootSignature(generateMipsPipeline->GetRootSignature());

	ID3D12DescriptorHeap* ppHeaps[] = { Application::Get().GetRenderer().GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetHeapPointer() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	GenerateMipsCB generateMipsCB {};
	generateMipsCB.IsSRGB = isSRGB;

	const auto resource = texture.GetResource();
	const auto resourceDesc = resource->GetDesc();

	TransitionBarrier(texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

	// for ( uint32_t i = 0; i < resourceDesc.MipLevels; i++)
	// {
	// 	texture.SetState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, i);
	// }

	for ( uint32_t srcMip = 0; srcMip < resourceDesc.MipLevels - 1; )
	{
		auto srcWidth = resourceDesc.Width >> srcMip;
		auto srcHeight = resourceDesc.Height >> srcMip;
		auto dstWidth = static_cast<uint32_t>( srcWidth >> 1 );
		auto dstHeight = srcHeight >> 1;

		// 0b00(0): Both width and height are even.
		// 0b01(1): Width is odd, height is even.
		// 0b10(2): Width is even, height is odd.
		// 0b11(3): Both width and height are odd.
		generateMipsCB.SrcDimension = ( srcHeight & 1 ) << 1 | ( srcWidth & 1 );

		uint32_t AdditionalMips;
		_BitScanForward(reinterpret_cast<unsigned long*>(&AdditionalMips),
			(dstWidth == 1 ? dstHeight : dstWidth) | (dstHeight == 1 ? dstWidth : dstHeight));
		uint32_t NumMips = 1 + (AdditionalMips > 3 ? 3 : AdditionalMips);
		if (srcMip + NumMips > resourceDesc.MipLevels)
			NumMips = resourceDesc.MipLevels - srcMip;

		// Dimensions should not reduce to 0.
		// This can happen if the width and height are not the same.
		dstWidth = std::max<DWORD>( 1, dstWidth );
		dstHeight = std::max<DWORD>( 1, dstHeight );

		generateMipsCB.SrcMipLevel = srcMip;
		generateMipsCB.NumMipLevels = NumMips;
		generateMipsCB.TexelSize.x = 1.0f / (float)dstWidth;
		generateMipsCB.TexelSize.y = 1.0f / (float)dstHeight;

		m_commandList->SetComputeRoot32BitConstants( 0, 6, &generateMipsCB, 0);

		TransitionBarrier(texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
		for ( uint32_t mip = 0; mip < NumMips; ++mip )
			m_commandList->SetComputeRootDescriptorTable(2 + mip, texture.GetUAV().GPU(srcMip + mip + 1));

		// This transition needs to happen AFTER the UAV transition.
		// The other way around will invalidate the SRV and cause
		// a validation layer error
		TransitionBarrier(texture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
		m_commandList->SetComputeRootDescriptorTable(1, texture.GetSRV().GPU());

		Dispatch((dstWidth + 7) / 8, (dstHeight + 7) / 8, 1);

		UAVBarrier(texture);

		srcMip += NumMips;
	}
}

void CommandBatch::TransitionBarrier(GpuResource& resource, const D3D12_RESOURCE_STATES after, const bool sendBarrier)
{
	const auto before = resource.GetState();

	if (m_listType == D3D12_COMMAND_LIST_TYPE_COMPUTE)
	{
		assert((before & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == before);
		assert((after & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == after);
	}

	if (before != after)
	{
		D3D12_RESOURCE_BARRIER& barrier = m_resourceBarriers[m_barrierIndex++];
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = resource.GetResource();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = before;
		barrier.Transition.StateAfter = after;

		resource.m_UsageState = after;
	} else if (after == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		UAVBarrier(resource, sendBarrier);

	if (sendBarrier || m_barrierIndex == 16)
		SendResourceBarriers();
}

void CommandBatch::AliasingBarrier(GpuResource& beforeResource, GpuResource& afterResource, const bool sendBarriers)
{
	D3D12_RESOURCE_BARRIER& barrier = m_resourceBarriers[m_barrierIndex++];
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
	barrier.Aliasing.pResourceBefore = beforeResource.GetResource();
	barrier.Aliasing.pResourceAfter = afterResource.GetResource();

	if (sendBarriers || m_barrierIndex == 16)
		SendResourceBarriers();
}

void CommandBatch::UAVBarrier(GpuResource& resource, const bool sendBarriers)
{
	D3D12_RESOURCE_BARRIER& barrier = m_resourceBarriers[m_barrierIndex++];
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.UAV.pResource = resource.GetResource();

	if (sendBarriers || m_barrierIndex == 16)
		SendResourceBarriers();
}

void CommandBatch::UploadToBuffer(GpuBuffer& buffer, const void* data, const size_t size, const uint32_t offset)
{
	auto& renderer = Application::Get().GetRenderer();
	CommandQueue& queue = renderer.GetCommandManager().GetQueue(m_listType);

	const BufferView bufferView = queue.AllocateUploadBufferView(size);

	memcpy(bufferView.data, data, size);

	const auto beforeState = buffer.GetState();

	TransitionBarrier(buffer, D3D12_RESOURCE_STATE_COPY_DEST, true);
	CopyBufferRegion(buffer, offset, *bufferView.buffer, bufferView.offset, size);
	TransitionBarrier(buffer, beforeState, true);

	m_usedBufferViews.push_back(bufferView);
}

void CommandBatch::CopyBuffer(GpuResource& dst, GpuResource& src)
{
	TransitionBarrier(dst, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionBarrier(src, D3D12_RESOURCE_STATE_COPY_SOURCE, true);

	m_commandList->CopyResource(dst.GetResource(), src.GetResource());
}

void CommandBatch::CopyBufferRegion(GpuBuffer& dst, uint64_t dstOffset, GpuBuffer& src, uint64_t srcOffset, uint64_t numBytes)
{
	TransitionBarrier(dst, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionBarrier(src, D3D12_RESOURCE_STATE_COPY_SOURCE, true);

	m_commandList->CopyBufferRegion(dst.GetResource(), dstOffset, src.GetResource(), srcOffset, numBytes);
}

void CommandBatch::SetRootParameter(const uint32_t idx, const DescriptorHandle& handle) const
{
	m_commandList->SetGraphicsRootDescriptorTable(idx, handle.GPU());
}

void CommandBatch::SetPipeline(const Pipeline& pipeline)
{
	if ((m_lastPipeline && m_lastPipeline->GetShader() != pipeline.GetShader()) || !m_lastPipeline)
	{
		m_commandList->SetPipelineState(pipeline.Get());
		m_commandList->SetGraphicsRootSignature(pipeline.GetRootSignature());
		m_lastPipeline = &pipeline;
	}
}

void CommandBatch::InsertTimeStamp(ID3D12QueryHeap* queryHeap, const uint32_t queryIdx) const
{
	m_commandList->EndQuery(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, queryIdx);
}

void CommandBatch::ResolveTimeStamps(ID3D12Resource* readbackHeap, ID3D12QueryHeap* queryHeap, const uint32_t numQueries) const
{
	m_commandList->ResolveQueryData(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, numQueries, readbackHeap, 0);
}

void CommandBatch::Dispatch(const uint32_t x, const uint32_t y, const uint32_t z) const
{
	m_commandList->Dispatch(x, y, z);
}

void CommandBatch::SendResourceBarriers()
{
	if (m_barrierIndex > 0)
		m_commandList->ResourceBarrier(m_barrierIndex, m_resourceBarriers);

	m_barrierIndex = 0;

}