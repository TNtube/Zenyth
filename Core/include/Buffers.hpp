#pragma once
#include "DescriptorHeap.hpp"
#include "Core.hpp"

namespace Zenyth {
	class GpuBuffer
	{
	public:
		GpuBuffer() = default;

		DELETE_COPY_CTOR(GpuBuffer)
		DEFAULT_MOVE_CTOR(GpuBuffer)

		~GpuBuffer() = default;

		void Create(ID3D12Device *device, const std::wstring& name, uint32_t numElements, uint32_t elementSize, const void* initialData = nullptr);
		[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

		void Map(UINT8** pDataBegin);
		void Unmap();

		[[nodiscard]] bool IsValid() const { return m_buffer != nullptr; }
		[[nodiscard]] uint32_t GetElementCount() const { return m_elementCount; }

	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource>	m_buffer {};
		ID3D12Device*							m_pDevice {};
		size_t									m_bufferSize {};
		uint32_t								m_elementCount {};
		uint32_t								m_elementSize {};
		bool									m_mapped {};
	};


	template<std::unsigned_integral TIndex>
	class IndexBuffer : public GpuBuffer
	{
	public:
		IndexBuffer() = default;

		DELETE_COPY_CTOR(IndexBuffer)
		DEFAULT_MOVE_CTOR(IndexBuffer)

		~IndexBuffer() = default;


		explicit IndexBuffer(std::vector<TIndex> indices) : m_indices(std::move(indices)) {}

		void Create(ID3D12Device *device, const std::wstring& name);
		void Create(ID3D12Device *device, const std::wstring& name, uint32_t numElements, uint32_t elementSize, const void* initialData = nullptr);
		void PushTriangle(TIndex v0, TIndex v1, TIndex v2);

		[[nodiscard]] bool IsValid() const { return m_indices.size() > 0 && GpuBuffer::IsValid(); }
		[[nodiscard]] const std::vector<TIndex>& GetIndices() const { return m_indices; }
		[[nodiscard]] const D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() const { return &m_ibv; }

		void Destroy() { m_indices.clear(); m_buffer.Reset(); }

	private:
		D3D12_INDEX_BUFFER_VIEW		m_ibv {};
		std::vector<TIndex>			m_indices {};
	};


	template<typename TVertex>
	class VertexBuffer : public GpuBuffer
	{
	public:
		VertexBuffer() = default;
		DELETE_COPY_CTOR(VertexBuffer)
		DEFAULT_MOVE_CTOR(VertexBuffer)

		~VertexBuffer() = default;

		explicit VertexBuffer(std::vector<TVertex> vertices) : m_vertices(std::move(vertices)) {}

		void Create(ID3D12Device *device, const std::wstring& name);
		void Create(ID3D12Device *device, const std::wstring& name, uint32_t numElements, uint32_t elementSize, const void* initialData = nullptr);
		uint32_t PushVertex(const TVertex& vertice);

		[[nodiscard]] bool IsValid() const { return m_vertices.size() > 0 && GpuBuffer::IsValid(); }
		[[nodiscard]] const D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() const { return &m_vbv; }

		void Destroy() { m_vertices.clear(); m_buffer.Reset(); }

	private:
		D3D12_VERTEX_BUFFER_VIEW	m_vbv {};
		std::vector<TVertex>		m_vertices {};
	};

	template<typename TConstant>
	class ConstantBuffer : protected GpuBuffer
	{
	public:
		ConstantBuffer() = default;

		DELETE_COPY_CTOR(ConstantBuffer)
		DEFAULT_MOVE_CTOR(ConstantBuffer)

		~ConstantBuffer();

		void Create(ID3D12Device *device, const std::wstring& name, DescriptorHeap& resourceHeap, uint32_t numElements = 1);

		void SetData(const TConstant& data, uint32_t frameIndex = 0);

		[[nodiscard]] const DescriptorHandle& GetDescriptorHandle(const uint32_t frameIndex = 0) const { return m_cbvHandles[frameIndex]; }


		using GpuBuffer::Map;
		using GpuBuffer::Unmap;

	private:
		DescriptorHeap*					m_resourceHeap {};
		std::vector<DescriptorHandle>	m_cbvHandles {};
		uint8_t*						m_mappedData {};
	};

	class DepthStencilBuffer : protected GpuBuffer
	{
	public:
		DepthStencilBuffer() = default;

		DELETE_COPY_CTOR(DepthStencilBuffer)
		DEFAULT_MOVE_CTOR(DepthStencilBuffer)

		~DepthStencilBuffer();

		// no need to re-instantiate the buffer for resizing it
		void Create(ID3D12Device *device, const std::wstring& name, DescriptorHeap& dsvHeap, uint32_t width, uint32_t height);

		[[nodiscard]] const DescriptorHandle& GetDescriptorHandle() const { return m_dsvHandle; }

	private:
		DescriptorHeap*		m_dsvHeap {};
		DescriptorHandle	m_dsvHandle {};
	};
}

#include "Buffers.inl"
