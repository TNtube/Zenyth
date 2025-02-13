#pragma once
#include "Core.hpp"
#include "stdafx.h"
#include <format>

namespace Zenyth {
	using Microsoft::WRL::ComPtr;

	class Buffer
	{
	public:
		Buffer(ID3D12Device* device, size_t size);

		[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

		void Map(UINT8** pDataBegin) const;
		void Unmap() const;

	protected:
		ComPtr<ID3D12Resource> m_buffer;
		ID3D12Device* m_pDevice {};
	};

	template<typename T>
	class VertexBuffer final : public Buffer
	{
	public:
		VertexBuffer(ID3D12Device* device, const T* data, size_t size);
		void Apply(ID3D12GraphicsCommandList* commandList) const;

	private:
		D3D12_VERTEX_BUFFER_VIEW m_bufferView {};
	};

	class IndexBuffer final : public Buffer
	{
	public:
		IndexBuffer(ID3D12Device* device, const uint32_t* data, size_t size);
		void Apply(ID3D12GraphicsCommandList* commandList) const;

	private:
		D3D12_INDEX_BUFFER_VIEW m_bufferView {};
	};

	template<typename T>
	class ConstantBuffer final : public Buffer
	{
	public:
		ConstantBuffer(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap, int8_t offset = 0);

		~ConstantBuffer() = default;

		ConstantBuffer(const ConstantBuffer&) = delete;
		ConstantBuffer& operator=(const ConstantBuffer&) = delete;

		ConstantBuffer(ConstantBuffer&&) = default;
		ConstantBuffer& operator=(ConstantBuffer&&) = default;

		void SetData(const T& data);
		void Apply(ID3D12GraphicsCommandList* commandList);

	private:
		ID3D12DescriptorHeap* m_pResourceHeap {};
		D3D12_GPU_DESCRIPTOR_HANDLE m_cbvGpuHandle {};

		UINT8* m_pCbvDataBegin {};

		bool m_mapped = false;
		int8_t m_offset = 0;
	};


	template<typename T>
	VertexBuffer<T>::VertexBuffer(ID3D12Device* device, const T* data, size_t size)
		: Buffer(device, size)
	{
		auto msg = std::format("Vertex buffer of size {} bytes and type {}", size, typeid(T).name());
		std::wstring wmsg(msg.begin(), msg.end());
		m_buffer->SetName(wmsg.c_str());
		UINT8* pVertexDataBegin;

		Map(&pVertexDataBegin);
		memcpy(pVertexDataBegin, data, size);
		Unmap();

		m_bufferView.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_bufferView.StrideInBytes = sizeof(T);
		m_bufferView.SizeInBytes = size;
	}

	template<typename T>
	void VertexBuffer<T>::Apply(ID3D12GraphicsCommandList* commandList) const
	{
		commandList->IASetVertexBuffers(0, 1, &m_bufferView);
	}


	template<class T>
	ConstantBuffer<T>::ConstantBuffer(ID3D12Device *device, ID3D12DescriptorHeap *descriptorHeap, const int8_t offset)
		: Buffer(device, sizeof(T)), m_pResourceHeap(descriptorHeap), m_offset(offset)
	{
		auto msg = std::format("Constant buffer of offset {} bytes and type {}", offset, typeid(T).name());
		std::wstring wmsg(msg.begin(), msg.end());
		m_buffer->SetName(wmsg.c_str());
		// Describe and create a constant buffer view.
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_buffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = sizeof(T);
		const CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(
			m_pResourceHeap->GetCPUDescriptorHandleForHeapStart(),
			m_offset, // Offset from start
			m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		);
		device->CreateConstantBufferView(&cbvDesc, cbvHandle);

		m_cbvGpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
				m_pResourceHeap->GetGPUDescriptorHandleForHeapStart(),
				m_offset, // Same offset as in CPU handle creation
				m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}

	template<class T>
	void ConstantBuffer<T>::SetData(const T &data) {
		if (!m_mapped) {
			Map(&m_pCbvDataBegin);
			m_mapped = true;
		}
		memcpy(m_pCbvDataBegin, &data, sizeof(data));
	}

	template<class T>
	void ConstantBuffer<T>::Apply(ID3D12GraphicsCommandList *commandList) {
		commandList->SetGraphicsRootDescriptorTable(m_offset, m_cbvGpuHandle);
	}
}
