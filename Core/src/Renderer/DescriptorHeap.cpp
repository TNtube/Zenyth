#include "pch.hpp"

#include "Renderer/DescriptorHeap.hpp"

#include "Application.hpp"
#include "Core.hpp"


namespace Zenyth
{
	using namespace DirectX;
	DescriptorHandle::DescriptorHandle(const D3D12_DESCRIPTOR_HEAP_TYPE type)
		: m_type(type), m_cpuHandle{ 0 }, m_gpuHandle{ 0 }
	{
	}

	DescriptorHandle::DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, const uint32_t size,
	                                   const D3D12_DESCRIPTOR_HEAP_TYPE type)
		: m_type(type), m_cpuHandle(cpuHandle), m_gpuHandle{ 0 }, m_descriptorSize(size)
	{
	}

	DescriptorHandle::DescriptorHandle(
		const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		const D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle,
		uint32_t size,
		const D3D12_DESCRIPTOR_HEAP_TYPE type)
		: m_type(type), m_cpuHandle(cpuHandle), m_gpuHandle(gpuHandle), m_descriptorSize(size)
	{
	}

	DescriptorHandle DescriptorHandle::operator+(const uint32_t offset) const
	{
		DescriptorHandle ret = *this;
		ret += offset;
		return ret;
	}

	void DescriptorHandle::operator+=(const uint32_t offset)
	{
		if (m_cpuHandle.ptr != 0)
			m_cpuHandle.ptr += offset * m_descriptorSize;
		if (m_gpuHandle.ptr != 0)
			m_gpuHandle.ptr += offset * m_descriptorSize;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle::CPU(const size_t idx) const
	{
		auto base = m_cpuHandle;
		base.ptr += idx * m_descriptorSize;

		return base;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHandle::GPU(const size_t idx) const
	{
		auto base = m_gpuHandle;
		base.ptr += idx * m_descriptorSize;

		return base;
	}

	DescriptorHeap::DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		m_heapDesc.Type = type;
	}

	void DescriptorHeap::Create(const std::wstring& debugHeapName, const uint32_t maxCount)
	{
		const auto device = Application::Get().GetRenderer().GetDevice();
		m_heapDesc.NumDescriptors = maxCount;
		m_heapDesc.Flags = m_heapDesc.Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || m_heapDesc.Type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV
			? D3D12_DESCRIPTOR_HEAP_FLAG_NONE
			: D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_heapDesc.NodeMask = 1;

		ThrowIfFailed(device->CreateDescriptorHeap(&m_heapDesc, IID_PPV_ARGS(m_heap.ReleaseAndGetAddressOf())));

#ifndef _DEBUG
		(void)debugHeapName;
#else
		SUCCEEDED(m_heap->SetName(debugHeapName.c_str()));
#endif

		const auto descriptorSize = device->GetDescriptorHandleIncrementSize(m_heapDesc.Type);
		m_firstHandle = DescriptorHandle(
			m_heap->GetCPUDescriptorHandleForHeapStart(),
			m_heapDesc.Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
				? m_heap->GetGPUDescriptorHandleForHeapStart()
				: D3D12_GPU_DESCRIPTOR_HANDLE{ 0 },
			descriptorSize,
			m_heapDesc.Type
);

		m_freeList.reserve(m_heapDesc.NumDescriptors);
		for (int n = 0; n < m_heapDesc.NumDescriptors; n++)
			m_freeList.push_back(n);
	}

	bool DescriptorHeap::HasAvailableSpace() const
	{
		return !m_freeList.empty();
	}

	DescriptorHandle DescriptorHeap::Alloc(const int64_t count)
	{
		assert(m_freeList.size() >= count && "Descriptor Heap out of space. Increase heap size.");

		if (count > 1)
			std::ranges::sort(m_freeList, std::less());

		if (count == 1)
		{
			const auto idx = m_freeList.back();
			m_freeList.pop_back();
			return m_firstHandle + idx;
		}

		uint32_t lastIdx = m_freeList.front();
		int consIdx = 0;
		for (auto it = m_freeList.begin() + 1; it != m_freeList.end(); ++it)
		{
			consIdx = *it - lastIdx == 1 ? consIdx + 1 : 0;
			lastIdx = *it;

			if (consIdx == count)
			{
				m_freeList.erase(it - count + 1, it + 1);
				return m_firstHandle + (lastIdx - count);
			}
		}

		throw std::exception("Descriptor Heap out of space. Increase heap size.");
	}

	void DescriptorHeap::Free(const DescriptorHandle& firstHandle, const int64_t count)
	{
		const int cpu_idx = static_cast<int>((firstHandle.GetCpuPtr() - m_firstHandle.GetCpuPtr()) / m_firstHandle.OffsetSize());

#ifndef NDEBUG
		if (firstHandle.IsShaderVisible())
		{
			const int gpu_idx = static_cast<int>((firstHandle.GetGpuPtr() - m_firstHandle.GetGpuPtr()) / m_firstHandle.OffsetSize());
			assert(cpu_idx == gpu_idx && "CPU and GPU indices do not match.");
		}
#endif

		for (int64_t i = 0; i < count; i++)
			m_freeList.push_back(cpu_idx + i);
	}

	DescriptorHandle DescriptorHeap::operator[](const uint32_t arrayIdx) const
	{
		return m_firstHandle + arrayIdx;
	}

	uint32_t DescriptorHeap::GetOffsetOfHandle(const DescriptorHandle& dHandle) const
	{
		return static_cast<uint32_t>(dHandle.GetCpuPtr() - m_firstHandle.GetCpuPtr()) / dHandle.OffsetSize();
	}

	bool DescriptorHeap::ValidateHandle(const DescriptorHandle& dHandle) const
	{
		if (dHandle.GetCpuPtr() < m_firstHandle.GetCpuPtr() ||
			dHandle.GetCpuPtr() >= m_firstHandle.GetCpuPtr() + m_heapDesc.NumDescriptors * m_firstHandle.OffsetSize())
			return false;

		if (!dHandle.IsShaderVisible())
			return true;

		if (dHandle.GetGpuPtr() - m_firstHandle.GetGpuPtr() !=
			dHandle.GetCpuPtr() - m_firstHandle.GetCpuPtr())
			return false;

		return true;
	}
}
