#include "pch.hpp"

#include "DescriptorHeap.hpp"
#include "Core.hpp"


namespace Zenyth
{
	using namespace DirectX;
	DescriptorHandle::DescriptorHandle() : m_cpuHandle{ 0 }, m_gpuHandle{ 0 }
	{
	}

	DescriptorHandle::DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
		: m_cpuHandle(cpuHandle), m_gpuHandle{ 0 }
	{
	}

	DescriptorHandle::DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, const D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
		: m_cpuHandle(cpuHandle), m_gpuHandle(gpuHandle)
	{
	}

	DescriptorHandle DescriptorHandle::operator+(const uint32_t offsetScaledByDescriptorSize) const
	{
		DescriptorHandle ret = *this;
		ret += offsetScaledByDescriptorSize;
		return ret;
	}

	void DescriptorHandle::operator+=(const uint32_t offsetScaledByDescriptorSize)
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
		m_firstHandle = DescriptorHandle(
			m_heap->GetCPUDescriptorHandleForHeapStart(),
			m_heapDesc.Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
				? m_heap->GetGPUDescriptorHandleForHeapStart()
				: D3D12_GPU_DESCRIPTOR_HANDLE{ 0 }
);
		m_nextFreeHandle = m_firstHandle;

		m_freeList.reserve(static_cast<int>(m_heapDesc.NumDescriptors));
		for (int n = static_cast<int>(m_heapDesc.NumDescriptors); n > 0; n--)
			m_freeList.push_back(n - 1);
	}

	bool DescriptorHeap::HasAvailableSpace() const
	{
		return !m_freeList.empty();
	}

	DescriptorHandle DescriptorHeap::Alloc()
	{
		assert(HasAvailableSpace() && "Descriptor Heap out of space.  Increase heap size.");

		const auto idx = m_freeList.back();
		m_freeList.pop_back();
		const DescriptorHandle ret = m_firstHandle + idx * m_descriptorSize;
		return ret;
	}

	void DescriptorHeap::Free(const DescriptorHandle &dHandle)
	{
		const int cpu_idx = static_cast<int>((dHandle.GetCpuPtr() - m_firstHandle.GetCpuPtr()) / m_descriptorSize);

#ifndef NDEBUG
		if (dHandle.IsShaderVisible())
		{
			const int gpu_idx = static_cast<int>((dHandle.GetGpuPtr() - m_firstHandle.GetGpuPtr()) / m_descriptorSize);
			assert(cpu_idx == gpu_idx && "CPU and GPU indices do not match.");
		}
#endif

		m_freeList.push_back(cpu_idx);
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

		if (!dHandle.IsShaderVisible())
			return true;

		if (dHandle.GetGpuPtr() - m_firstHandle.GetGpuPtr() !=
			dHandle.GetCpuPtr() - m_firstHandle.GetCpuPtr())
			return false;

		return true;
	}
}
