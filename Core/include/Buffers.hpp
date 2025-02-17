#pragma once
#include "DescriptorHeap.hpp"

namespace Zenyth {
	using Microsoft::WRL::ComPtr;

	class Buffer
	{
	public:
		void Create(ID3D12Device *device, const std::wstring& name, uint32_t numElements, uint32_t elementSize, const void* initialData = nullptr);
		[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

		void Map(UINT8** pDataBegin);
		void Unmap();

		[[nodiscard]] bool IsValid() const { return m_buffer != nullptr; }
		[[nodiscard]] uint32_t GetElementCount() const { return m_elementCount; }

	protected:
		ComPtr<ID3D12Resource>	m_buffer {};
		ID3D12Device*			m_pDevice {};
		size_t					m_bufferSize {};
		uint32_t				m_elementCount {};
		uint32_t				m_elementSize {};
		bool					m_mapped {};
	};


	template<std::unsigned_integral TIndex>
	class IndexBuffer : public Buffer
	{
	public:
		IndexBuffer() = default;
		explicit IndexBuffer(std::vector<TIndex> indices) : m_indices(std::move(indices)) {}

		void Create(ID3D12Device *device, const std::wstring& name);
		void Create(ID3D12Device *device, const std::wstring& name, uint32_t numElements, uint32_t elementSize, const void* initialData = nullptr);
		void PushTriangle(TIndex v0, TIndex v1, TIndex v2);

		[[nodiscard]] bool IsValid() const { return m_indices.size() > 0 && Buffer::IsValid(); }
		[[nodiscard]] const std::vector<TIndex>& GetIndices() const { return m_indices; }
		[[nodiscard]] const D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() const { return &m_ibv; }

		void Destroy() { m_indices.clear(); m_buffer.Reset(); }

	private:
		D3D12_INDEX_BUFFER_VIEW m_ibv {};
		std::vector<TIndex> m_indices {};
	};


	template<typename TVertex>
	class VertexBuffer : public Buffer
	{
	public:
		VertexBuffer() = default;
		explicit VertexBuffer(std::vector<TVertex> vertices) : m_vertices(std::move(vertices)) {}

		void Create(ID3D12Device *device, const std::wstring& name);
		void Create(ID3D12Device *device, const std::wstring& name, uint32_t numElements, uint32_t elementSize, const void* initialData = nullptr);
		uint32_t PushVertex(const TVertex& vertice);

		[[nodiscard]] bool IsValid() const { return m_vertices.size() > 0 && Buffer::IsValid(); }
		[[nodiscard]] const D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() const { return &m_vbv; }

		void Destroy() { m_vertices.clear(); m_buffer.Reset(); }

	private:
		D3D12_VERTEX_BUFFER_VIEW m_vbv {};
		std::vector<TVertex> m_vertices {};
	};

	template<typename TConstant>
	class ConstantBuffer : protected Buffer
	{
	public:
		void Create(ID3D12Device *device, const std::wstring& name, DescriptorHeap& resourceHeap);
		void Create(ID3D12Device *device, const std::wstring& name, DescriptorHeap& resourceHeap, const TConstant& initialData);

		void SetData(const TConstant& data);

		[[nodiscard]] const DescriptorHandle& GetDescriptorHandle() const { return m_cbvHandle; }


		using Buffer::Map;
		using Buffer::Unmap;

	private:
		DescriptorHandle m_cbvHandle {};
		uint8_t* m_mappedData {};
	};
}

#include "Buffers.inl"
