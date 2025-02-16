#pragma once

namespace Zenyth
{
	class DescriptorHandle
	{
	public:
		DescriptorHandle();
		explicit DescriptorHandle( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle );
		DescriptorHandle( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle );

		DescriptorHandle operator+ (INT offsetScaledByDescriptorSize ) const;

		void operator += (INT offsetScaledByDescriptorSize );

		const D3D12_CPU_DESCRIPTOR_HANDLE* operator&() const { return &m_cpuHandle; }
		explicit operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return m_cpuHandle; }
		explicit operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return m_gpuHandle; }

		size_t GetCpuPtr() const { return m_cpuHandle.ptr; }
		uint64_t GetGpuPtr() const { return m_gpuHandle.ptr; }
		D3D12_CPU_DESCRIPTOR_HANDLE CPU() const { return m_cpuHandle; }
		D3D12_GPU_DESCRIPTOR_HANDLE GPU() const { return m_gpuHandle; }
		bool IsNull() const { return m_cpuHandle.ptr == 0; }
		bool IsShaderVisible() const { return m_gpuHandle.ptr != 0; }

	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
	};

	class DescriptorHeap
	{
	public:
		DescriptorHeap() = default;
		~DescriptorHeap() { Destroy(); }

		void Create(ID3D12Device *device, const std::wstring& debugHeapName, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t maxCount);
		void Destroy() { m_heap = nullptr; }

		bool HasAvailableSpace( uint32_t count ) const;
		DescriptorHandle Alloc( uint32_t count = 1 );

		DescriptorHandle GetStartHandle() const { return m_firstHandle; }

		DescriptorHandle operator[] (uint32_t arrayIdx) const;
		uint32_t GetOffsetOfHandle(const DescriptorHandle& dHandle ) const;
		ID3D12DescriptorHeap* GetHeapPointer() const { return m_heap.Get(); }
		uint32_t GetDescriptorSize() const { return m_descriptorSize; }

		bool ValidateHandle( const DescriptorHandle& dHandle ) const;

	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
		D3D12_DESCRIPTOR_HEAP_DESC m_heapDesc;
		uint32_t m_descriptorSize;
		uint32_t m_numFreeDescriptors;
		DescriptorHandle m_firstHandle;
		DescriptorHandle m_nextFreeHandle;
	};
}
