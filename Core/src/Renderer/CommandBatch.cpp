#include "pch.hpp"

#include "Renderer/CommandBatch.hpp"

#include "Application.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/Renderer.hpp"

namespace Zenyth {
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

		const auto fence = queue.ExecuteCommandList(m_commandList.Get());

		queue.FreeAllocator(fence, m_commandAllocator);

		m_commandList.Reset();
		m_commandList = nullptr;

		if (wait)
			queue.WaitForFence(fence);

		return fence;
	}

	void CommandBatch::InitializeBuffer(GpuBuffer& buffer, const void *data, const size_t size, const size_t offset)
	{
		CommandBatch batch = Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);

		auto& renderer = Application::Get().GetRenderer();
		CommandQueue& queue = renderer.GetCommandManager().GetQueue(batch.m_listType);

		const BufferView bufferView = queue.AllocateUploadBufferView(size);

		memcpy(bufferView.data, data, size);

		batch.TransitionBarrier(&buffer, D3D12_RESOURCE_STATE_COPY_DEST, true);
		batch.CopyBufferRegion(buffer, offset, *bufferView.buffer, bufferView.offset, size);
		batch.TransitionBarrier(&buffer, D3D12_RESOURCE_STATE_GENERIC_READ, true);

		const auto fence = batch.End(true);

		queue.FreeUploadBufferView(fence, bufferView);
	}

	void CommandBatch::InitializeTexture(Texture& texture, const size_t subresourceCount, const D3D12_SUBRESOURCE_DATA* subData)
	{
		const auto uploadBufferSize = GetRequiredIntermediateSize(texture.GetResource(), 0, subresourceCount);

		CommandBatch batch = Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);

		auto& renderer = Application::Get().GetRenderer();
		CommandQueue& queue = renderer.GetCommandManager().GetQueue(batch.m_listType);

		const BufferView bufferView = queue.AllocateUploadBufferView(uploadBufferSize);

		UpdateSubresources(batch.m_commandList.Get(), texture.GetResource(), bufferView.buffer->GetResource(), bufferView.offset, 0, subresourceCount, subData);
		batch.TransitionBarrier(&texture, D3D12_RESOURCE_STATE_GENERIC_READ, true);

		const auto fence = batch.End(true);

		queue.FreeUploadBufferView(fence, bufferView);
	}

	void CommandBatch::GenerateMips(Texture& texture)
	{
		auto device = Application::Get().GetRenderer().GetDevice();
		GpuResource* resource = &texture;
		const auto resourceDesc = resource->GetResource()->GetDesc();

		if ( resourceDesc.MipLevels == 1 )
			return;

		if ( resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D
			|| resourceDesc.DepthOrArraySize != 1
			|| resourceDesc.SampleDesc.Count > 1 )
		{
			throw std::exception( "GenerateMips is only supported for non-multi-sampled 2D Textures." );
		}

		CommandBatch batch = Begin(D3D12_COMMAND_LIST_TYPE_COMPUTE);

		auto uavResource = resource;
		GpuResource aliasResource;

		if (!texture.CheckUAVSupport() || ( resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS ) == 0)
		{
			auto aliasDesc = resourceDesc;
			aliasDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			aliasDesc.Flags &= ~(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

			auto uavDesc = aliasDesc;   // The flags for the UAV description must match that of the alias description.
			uavDesc.Format = Texture::GetUAVCompatibleFormat(resourceDesc.Format);

			D3D12_RESOURCE_DESC resourceDescs[] = {
				aliasDesc,
				uavDesc
			};

			// Create a heap that is large enough to store a copy of the original resource.
			auto allocationInfo = device->GetResourceAllocationInfo(0, _countof(resourceDescs), resourceDescs );

			D3D12_HEAP_DESC heapDesc = {};
			heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
			heapDesc.Alignment = allocationInfo.Alignment;
			heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
			heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;

			Microsoft::WRL::ComPtr<ID3D12Heap> heap;
			ThrowIfFailed(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)));

			// Create a placed resource that matches the description of the
			// original resource. This resource is used to copy the original
			// texture to the UAV compatible resource.
			ThrowIfFailed(device->CreatePlacedResource(
				heap.Get(),
				0,
				&aliasDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(aliasResource.GetAddressOf())
			));

			// Make sure the heap does not go out of scope until the command list
			// is finished executing on the command queue.
			batch.TrackResource(heap);


			// Create a placed resource that matches the description of the
			// original resource. This resource is used to copy the original
			// texture to the UAV compatible resource.
			ThrowIfFailed(device->CreatePlacedResource(
				heap.Get(),
				0,
				&aliasDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(aliasResource.GetAddressOf())
			));

			// Ensure the scope of the alias resource.
			batch.TrackResource(aliasResource.GetResource());

			// Create a UAV compatible resource in the same heap as the alias
			// resource.
			ThrowIfFailed(device->CreatePlacedResource(
				heap.Get(),
				0,
				&uavDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(uavResource->GetAddressOf())
			));


			batch.TrackResource(uavResource->GetResource());

			// Add an aliasing barrier for the alias resource.
			batch.AliasingBarrier(nullptr, &aliasResource);

			// Copy the original resource to the alias resource.
			// This ensures GPU validation.
			batch.CopyResource(&aliasResource, &texture);

			// Add an aliasing barrier for the UAV compatible resource.
			batch.AliasingBarrier(&aliasResource, uavResource);
		}

		// Generate mips with the UAV compatible resource.
		// auto uavTexture = device.CreateTexture( uavResource );
		// GenerateMips_UAV( uavTexture, Texture::IsSRGBFormat( resourceDesc.Format ) );

		if ( aliasResource.GetResource() )
		{
			batch.AliasingBarrier( uavResource, &aliasResource );
			// Copy the alias resource back to the original resource.
			batch.CopyResource( resource, &aliasResource );
		}


		batch.End();
	}

	void CommandBatch::TransitionBarrier(GpuResource* resource, const D3D12_RESOURCE_STATES after, const bool sendBarrier)
	{
		assert(resource);
		TransitionBarrier(resource->GetResource(), resource->m_UsageState, after, sendBarrier);

		resource->m_UsageState = after;
	}

	void CommandBatch::TransitionBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, bool sendBarrier)
	{
		if (before != after)
		{
			D3D12_RESOURCE_BARRIER& barrier = m_resourceBarriers[m_barrierIndex++];
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = resource;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = before;
			barrier.Transition.StateAfter = after;
		}

		if (sendBarrier || m_barrierIndex == 16)
		{
			SendResourceBarriers();
		}
	}

	void CommandBatch::CopyResource(GpuResource* dst, GpuResource* src)
	{
		TransitionBarrier(dst, D3D12_RESOURCE_STATE_COPY_DEST);
		TransitionBarrier(src, D3D12_RESOURCE_STATE_COPY_SOURCE, true);

		m_commandList->CopyResource(dst->GetResource(), src->GetResource());
	}

	void CommandBatch::CopyBufferRegion(GpuBuffer& dst, uint64_t dstOffset, GpuBuffer& src, uint64_t srcOffset, uint64_t numBytes)
	{
		TransitionBarrier(&dst, D3D12_RESOURCE_STATE_COPY_DEST);
		TransitionBarrier(&src, D3D12_RESOURCE_STATE_COPY_SOURCE, true);

		m_commandList->CopyBufferRegion(dst.GetResource(), dstOffset, src.GetResource(), srcOffset, numBytes);
	}

	void CommandBatch::SubmitMaterial(std::shared_ptr<Material> material)
	{
		m_currentMaterial = std::move(material);

		m_currentMaterial->Submit(m_commandList.Get());

		for (const auto& [idx, handle] : m_rootParameters)
			m_commandList->SetGraphicsRootDescriptorTable(idx, handle.GPU());
	}

	void CommandBatch::SetRootParameter(const uint32_t idx, const DescriptorHandle& handle)
	{
		if (m_currentMaterial)
			m_commandList->SetGraphicsRootDescriptorTable(idx, handle.GPU());
		else
			m_rootParameters.emplace_back(idx, handle);

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

	void CommandBatch::TrackResource(const Microsoft::WRL::ComPtr<ID3D12Object>& object)
	{
		m_trackedObjects.push_back(object);
	}

	void CommandBatch::TrackResource(const GpuBuffer& buffer)
	{
		assert(buffer.GetResource());

		TrackResource(buffer.GetResource());
	}

	void CommandBatch::AliasingBarrier(GpuResource* beforeResource, GpuResource* afterResource, bool sendBarriers)
	{
		D3D12_RESOURCE_BARRIER& barrier = m_resourceBarriers[m_barrierIndex++];
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
		barrier.Aliasing.pResourceBefore = beforeResource->GetResource();
		barrier.Aliasing.pResourceAfter = afterResource->GetResource();


		if (sendBarriers || m_barrierIndex == 16)
		{
			SendResourceBarriers();
		}
	}
}
