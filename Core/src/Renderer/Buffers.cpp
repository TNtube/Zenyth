#include <utility>

#include "pch.hpp"

#include "Core.hpp"
#include "Renderer/Buffers.hpp"

#include "Application.hpp"
#include "Renderer/CommandBatch.hpp"
#include "Renderer/Renderer.hpp"

namespace Zenyth
{
	void GpuBuffer::Create( const std::wstring& name, const uint32_t numElements, uint32_t elementSize, const void* initialData, bool align)
	{
		Destroy();

		m_elementCount = numElements;
		m_elementSize = align ? Math::AlignToMask(elementSize, 256u) : elementSize;;
		m_bufferSize = numElements * m_elementSize;

		const D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();

		m_UsageState = D3D12_RESOURCE_STATE_COMMON;

		GpuResource::Create(ResourceDesc, nullptr);

		m_GpuVirtualAddress = m_resource->GetGPUVirtualAddress();

		if (initialData)
			CommandBatch::InitializeBuffer(*this, initialData, m_bufferSize);

	#ifdef NDEBUG
		(name);
	#else
		m_resource->SetName(name.c_str());
	#endif

		CreateViews();
}

	void GpuBuffer::CreatePlaced(const std::wstring& name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset,
		uint32_t NumElements, uint32_t ElementSize, const void* initialData)
	{
		const auto device = Application::Get().GetRenderer().GetDevice();

		m_elementCount = NumElements;
		m_elementSize = ElementSize;
		m_bufferSize = NumElements * ElementSize;

		D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();

		m_UsageState = D3D12_RESOURCE_STATE_COMMON;

		ThrowIfFailed(device->CreatePlacedResource(pBackingHeap, HeapOffset, &ResourceDesc, m_UsageState, nullptr, IID_PPV_ARGS(&m_resource)));

		m_GpuVirtualAddress = m_resource->GetGPUVirtualAddress();

		if (initialData)
			CommandBatch::InitializeBuffer(*this, initialData, m_bufferSize);

#ifdef NDEBUG
		(name);
#else
		m_resource->SetName(name.c_str());
#endif

		CreateViews();
	}

	void GpuBuffer::Destroy()
	{
		GpuResource::Destroy();

		if (!m_UAV.IsNull())
			Application::Get().GetRenderer().FreeDescriptor(m_UAV);
		
		if (!m_SRV.IsNull())
			Application::Get().GetRenderer().FreeDescriptor(m_SRV);
		

	}


	D3D12_RESOURCE_DESC GpuBuffer::DescribeBuffer() const
	{
		assert(m_bufferSize != 0);

		D3D12_RESOURCE_DESC Desc = {};
		Desc.Alignment = 0;
		Desc.DepthOrArraySize = 1;
		Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		Desc.Flags = m_resourceFlags;
		Desc.Format = DXGI_FORMAT_UNKNOWN;
		Desc.Height = 1;
		Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		Desc.MipLevels = 1;
		Desc.SampleDesc.Count = 1;
		Desc.SampleDesc.Quality = 0;
		Desc.Width = m_bufferSize;
		return Desc;
	}

	void UploadBuffer::Create(const std::wstring& name, const size_t size)
	{
		Destroy();

		const auto& renderer = Application::Get().GetRenderer();
		const auto device = renderer.GetDevice();

		m_bufferSize = Math::AlignToMask(size, 256ull);

		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		const auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize);

		// Check device validity
		HRESULT deviceRemoved = device->GetDeviceRemovedReason();
		if (FAILED(deviceRemoved))
		{
			OutputDebugStringA("Device was removed!\n");
		}

		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_resource.GetAddressOf())));

		SUCCEEDED(m_resource->SetName(name.c_str()));
	}

	void* UploadBuffer::Map()
	{
		const CD3DX12_RANGE readRange(0, 0);
		SUCCEEDED(m_resource->Map(0, &readRange, &m_memory));
		return m_memory;
	}

	void UploadBuffer::Unmap() const
	{
		m_resource->Unmap(0, nullptr);
	}


	void VertexBuffer::CreateViews()
	{
		m_vbv.BufferLocation = GetGpuVirtualAddress();
		m_vbv.StrideInBytes = m_elementSize;
		m_vbv.SizeInBytes = m_bufferSize;
	}

	void IndexBuffer::CreateViews()
	{
		m_ibv.BufferLocation = GetGpuVirtualAddress();
		m_ibv.Format = DXGI_FORMAT_R32_UINT;
		m_ibv.SizeInBytes = m_bufferSize;
	}

	void ConstantBuffer::Destroy()
	{
		GpuBuffer::Destroy();
		if (!m_cbvHandle.IsNull())
			Application::Get().GetRenderer().FreeDescriptor(m_cbvHandle, m_elementCount);
	}

	void ConstantBuffer::CreateViews()
	{
		auto& renderer = Application::Get().GetRenderer();
		const auto device = renderer.GetDevice();

		if (m_cbvHandle.IsNull())
			m_cbvHandle = renderer.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_elementCount);

		for (int i = 0; std::cmp_less(i, m_elementCount); i++)
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = GetGpuVirtualAddress() + i * m_elementSize;
			cbvDesc.SizeInBytes = m_elementSize;
			device->CreateConstantBufferView(&cbvDesc, m_cbvHandle.CPU(i));
		}
	}

	void StructuredBuffer::CreateViews()
	{
		auto& renderer = Application::Get().GetRenderer();
		const auto device = renderer.GetDevice();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = m_elementCount;
		srvDesc.Buffer.StructureByteStride = m_elementSize;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		if (m_SRV.IsNull())
			m_SRV = renderer.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_SRV.CPU());

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.NumElements = m_elementCount;
		uavDesc.Buffer.StructureByteStride = m_elementSize;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		if (m_UAV.IsNull())
			m_UAV = renderer.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		device->CreateUnorderedAccessView(m_resource.Get(), nullptr, &uavDesc, m_UAV.CPU());
	}
}
