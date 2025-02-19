#include "pch.hpp"

#include "DescriptorHeap.hpp"
#include "Core.hpp"


namespace Zenyth
{
	using namespace DirectX;
	DescriptorHandle::DescriptorHandle()
	{
		m_cpuHandle.ptr = 0;
		m_gpuHandle.ptr = 0;
	}

	DescriptorHandle::DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
		: m_cpuHandle(cpuHandle)
	{
		m_gpuHandle.ptr = 0;
	}

	DescriptorHandle::DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, const D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
		: m_cpuHandle(cpuHandle), m_gpuHandle(gpuHandle)
	{
	}

	DescriptorHandle DescriptorHandle::operator+(const INT offsetScaledByDescriptorSize) const
	{
		DescriptorHandle ret = *this;
		ret += offsetScaledByDescriptorSize;
		return ret;
	}

	void DescriptorHandle::operator+=(const INT offsetScaledByDescriptorSize)
	{
		if (m_cpuHandle.ptr != 0)
			m_cpuHandle.ptr += offsetScaledByDescriptorSize;
		if (m_gpuHandle.ptr != 0)
			m_gpuHandle.ptr += offsetScaledByDescriptorSize;
	}




	void DescriptorHeap::Create(ID3D12Device* device, const std::wstring &debugHeapName, const D3D12_DESCRIPTOR_HEAP_TYPE type, const uint32_t maxCount)
	{
		m_heapDesc.Type = type;
		m_heapDesc.NumDescriptors = maxCount;
		m_heapDesc.Flags = type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV
			? D3D12_DESCRIPTOR_HEAP_FLAG_NONE
			: D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_heapDesc.NodeMask = 1;

		ThrowIfFailed(device->CreateDescriptorHeap(&m_heapDesc, IID_PPV_ARGS(m_heap.ReleaseAndGetAddressOf())));

#ifndef _DEBUG
		(void)debugHeapName;
#else
		SUCCEEDED(m_heap->SetName(debugHeapName.c_str()));
#endif

		m_descriptorSize = device->GetDescriptorHandleIncrementSize(m_heapDesc.Type);
		m_numFreeDescriptors = m_heapDesc.NumDescriptors;
		m_firstHandle = DescriptorHandle(
			m_heap->GetCPUDescriptorHandleForHeapStart(),
			m_heapDesc.Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
				? m_heap->GetGPUDescriptorHandleForHeapStart()
				: D3D12_GPU_DESCRIPTOR_HANDLE{ 0 }
);
		m_nextFreeHandle = m_firstHandle;
	}

	bool DescriptorHeap::HasAvailableSpace(const uint32_t count) const
	{
		return count <= m_numFreeDescriptors;
	}

	DescriptorHandle DescriptorHeap::Alloc(const uint32_t count)
	{
		assert(HasAvailableSpace(count), "Descriptor Heap out of space.  Increase heap size.");
		const DescriptorHandle ret = m_nextFreeHandle;
		m_nextFreeHandle += count * m_descriptorSize;
		m_numFreeDescriptors -= count;
		return ret;
	}

	DescriptorHandle DescriptorHeap::operator[](const uint32_t arrayIdx) const
	{
		return m_firstHandle + arrayIdx * m_descriptorSize;
	}

	uint32_t DescriptorHeap::GetOffsetOfHandle(const DescriptorHandle &dHandle) const
	{
		return static_cast<uint32_t>(dHandle.GetCpuPtr() - m_firstHandle.GetCpuPtr()) / m_descriptorSize;
	}

	bool DescriptorHeap::ValidateHandle(const DescriptorHandle &dHandle) const
	{
		if (dHandle.GetCpuPtr() < m_firstHandle.GetCpuPtr() ||
			dHandle.GetCpuPtr() >= m_firstHandle.GetCpuPtr() + m_heapDesc.NumDescriptors * m_descriptorSize)
			return false;

		if (dHandle.GetGpuPtr() - m_firstHandle.GetGpuPtr() !=
			dHandle.GetCpuPtr() - m_firstHandle.GetCpuPtr())
			return false;

		return true;
	}
}
