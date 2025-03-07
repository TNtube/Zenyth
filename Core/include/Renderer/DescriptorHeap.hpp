#pragma once
#include "Core.hpp"

namespace Zenyth
{
	class DescriptorHandle
	{
	public:
		DescriptorHandle();
		explicit DescriptorHandle( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle );
		DescriptorHandle( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle );

		DescriptorHandle operator+ (uint32_t offsetScaledByDescriptorSize ) const;

		void operator += (uint32_t offsetScaledByDescriptorSize );

		explicit operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return m_cpuHandle; }
		explicit operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return m_gpuHandle; }

		[[nodiscard]] size_t GetCpuPtr() const { return m_cpuHandle.ptr; }
		[[nodiscard]] uint64_t GetGpuPtr() const { return m_gpuHandle.ptr; }
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE CPU() const { return m_cpuHandle; }
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GPU() const { return m_gpuHandle; }
		[[nodiscard]] bool IsNull() const { return m_cpuHandle.ptr == 0; }
		[[nodiscard]] bool IsShaderVisible() const { return m_gpuHandle.ptr != 0; }

	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
	};

	class DescriptorHeap
	{
	public:
		DescriptorHeap() = default;

		DELETE_COPY_CTOR(DescriptorHeap)
		DEFAULT_MOVE_CTOR(DescriptorHeap)

		~DescriptorHeap() { Destroy(); }

		void Create(ID3D12Device *device, const std::wstring& debugHeapName, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t maxCount);
		void Destroy() { m_heap.Reset(); }

		[[nodiscard]] bool HasAvailableSpace() const;
		DescriptorHandle Alloc();
		void Free( const DescriptorHandle& dHandle );

		[[nodiscard]] DescriptorHandle GetStartHandle() const { return m_firstHandle; }

		DescriptorHandle operator[] (uint32_t arrayIdx) const;
		[[nodiscard]] uint32_t GetOffsetOfHandle(const DescriptorHandle& dHandle ) const;
		[[nodiscard]] ID3D12DescriptorHeap* GetHeapPointer() const { return m_heap.Get(); }
		[[nodiscard]] uint32_t GetDescriptorSize() const { return m_descriptorSize; }

		[[nodiscard]] bool ValidateHandle( const DescriptorHandle& dHandle ) const;

	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
		D3D12_DESCRIPTOR_HEAP_DESC m_heapDesc {};
		uint32_t m_descriptorSize {};
		DescriptorHandle m_firstHandle;
		DescriptorHandle m_nextFreeHandle;

		std::vector<uint32_t> m_freeList;
	};
}
