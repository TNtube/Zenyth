#pragma once
#include "Core.hpp"
#include "stdafx.h"

namespace Zenyth {
	using Microsoft::WRL::ComPtr;

	class Buffer {
	public:
		Buffer(ID3D12Device* device, size_t size);

		[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

		void Map(UINT8** pDataBegin) const;
		void Unmap() const;
	private:
		ComPtr<ID3D12Resource> m_buffer;
		ID3D12Device* m_pDevice {};
	};

	template<class T>
	class ConstantBuffer final {
	public:
		ConstantBuffer(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap, int8_t offset = 0);

		~ConstantBuffer() = default;

		ConstantBuffer(const ConstantBuffer&) = delete;
		ConstantBuffer& operator=(const ConstantBuffer&) = delete;

		ConstantBuffer(ConstantBuffer&&) = default;
		ConstantBuffer& operator=(ConstantBuffer&&) = default;

		void SetData(const T& data);
		void Apply(ID3D12GraphicsCommandList* commandList) const;

	private:
		Buffer m_buffer;

		ID3D12Device* m_pDevice {};
		ID3D12DescriptorHeap* m_pResourceHeap {};

		D3D12_GPU_DESCRIPTOR_HANDLE m_cbvGpuHandle {};

		UINT8* m_pCbvDataBegin {};

		bool m_mapped = false;
		int8_t m_offset = 0;
	};


	template<class T>
	ConstantBuffer<T>::ConstantBuffer(ID3D12Device *device, ID3D12DescriptorHeap *descriptorHeap, const int8_t offset)
		: m_buffer(device, sizeof(T)), m_pDevice(device), m_pResourceHeap(descriptorHeap),
		  m_offset(offset)
	{
		// Describe and create a constant buffer view.
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_buffer.GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = sizeof(T);
		const CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(
			descriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			offset, // Offset from start
			device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		);
		device->CreateConstantBufferView(&cbvDesc, cbvHandle);

		// Map and initialize the constant buffer. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		m_buffer.Map(&m_pCbvDataBegin);
		m_mapped = true;

		m_cbvGpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
				m_pResourceHeap->GetGPUDescriptorHandleForHeapStart(),
				m_offset, // Same offset as in CPU handle creation
				m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}

	template<class T>
	void ConstantBuffer<T>::SetData(const T &data) {
		memcpy(m_pCbvDataBegin, &data, sizeof(data));
	}

	template<class T>
	void ConstantBuffer<T>::Apply(ID3D12GraphicsCommandList *commandList) const {
		commandList->SetGraphicsRootDescriptorTable(m_offset, m_cbvGpuHandle);
	}
}
