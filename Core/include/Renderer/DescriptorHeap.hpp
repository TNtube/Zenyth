#pragma once
#include "Core.hpp"


class DescriptorHandle
{
public:
	explicit DescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES);
	DescriptorHandle( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, uint32_t size, D3D12_DESCRIPTOR_HEAP_TYPE type );
	DescriptorHandle( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, uint32_t size, D3D12_DESCRIPTOR_HEAP_TYPE type );

	DescriptorHandle operator+ (uint32_t offset ) const;
	void operator += (uint32_t offset );

	explicit operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return m_cpuHandle; }
	explicit operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return m_gpuHandle; }

	[[nodiscard]] size_t GetCpuPtr() const { return m_cpuHandle.ptr; }
	[[nodiscard]] uint64_t GetGpuPtr() const { return m_gpuHandle.ptr; }
	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE CPU(size_t idx = 0) const;
	[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GPU(size_t idx = 0) const;
	[[nodiscard]] bool IsNull() const { return m_cpuHandle.ptr == 0; }
	[[nodiscard]] bool IsShaderVisible() const { return m_gpuHandle.ptr != 0; }
	[[nodiscard]] D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return m_type; }
	[[nodiscard]] uint32_t OffsetSize() const { return m_descriptorSize; }

private:
	D3D12_DESCRIPTOR_HEAP_TYPE m_type = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
	uint32_t m_descriptorSize {};
};

class DescriptorHeap
{
public:
	explicit DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);
	DELETE_COPY_CTOR(DescriptorHeap)
	DEFAULT_MOVE_CTOR(DescriptorHeap)

	~DescriptorHeap() { Destroy(); }

	void Create(const std::wstring& debugHeapName, uint32_t maxCount);
	void Destroy() { m_heap.Reset(); }

	[[nodiscard]] bool HasAvailableSpace() const;
	DescriptorHandle Alloc(int64_t count = 1);
	void Free( const DescriptorHandle& firstHandle, int64_t count = 1);

	[[nodiscard]] DescriptorHandle GetStartHandle() const { return m_firstHandle; }

	DescriptorHandle operator[] (uint32_t arrayIdx) const;
	[[nodiscard]] uint32_t GetOffsetOfHandle(const DescriptorHandle& dHandle ) const;
	[[nodiscard]] ID3D12DescriptorHeap* GetHeapPointer() const { return m_heap.Get(); }
	[[nodiscard]] uint32_t GetDescriptorSize() const { return m_descriptorSize; }

	[[nodiscard]] bool ValidateHandle( const DescriptorHandle& dHandle ) const;

private:
	friend class Renderer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
	D3D12_DESCRIPTOR_HEAP_DESC m_heapDesc {};
	uint32_t m_descriptorSize {};
	DescriptorHandle m_firstHandle;

	std::vector<uint32_t> m_freeList;
};