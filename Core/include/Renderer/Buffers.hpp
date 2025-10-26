#pragma once
#include "DescriptorHeap.hpp"
#include "Core.hpp"

#include "GpuResource.hpp"

namespace Zenyth {

	class UploadBuffer;

	class GpuBuffer : public GpuResource
	{
	public:
		DELETE_COPY_CTOR(GpuBuffer)
		DEFAULT_MOVE_CTOR(GpuBuffer)

		void Create( const std::wstring& name, uint32_t numElements, uint32_t ElementSize,
			const void* initialData = nullptr, bool align = false );
		void CreatePlaced(const std::wstring& name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset, uint32_t NumElements, uint32_t ElementSize,
			const void* initialData = nullptr);

		void Destroy() override;

		[[nodiscard]] virtual bool IsValid() const { return m_resource != nullptr; }
		[[nodiscard]] uint32_t GetElementCount() const { return m_elementCount; }
		[[nodiscard]] uint32_t GetElementSize() const { return m_elementSize; }
		[[nodiscard]] size_t GetBufferSize() const { return m_bufferSize; }

		[[nodiscard]] DescriptorHandle GetUAV() const { return m_UAV; }
		[[nodiscard]] DescriptorHandle GetSRV() const { return m_SRV; }

	protected:
		GpuBuffer() : m_resourceFlags(D3D12_RESOURCE_FLAG_NONE) {}

		virtual void CreateViews() = 0;

		size_t                                 m_bufferSize {};
		uint32_t                               m_elementCount {};
		uint32_t                               m_elementSize {};

		DescriptorHandle                       m_UAV;
		DescriptorHandle                       m_SRV;

		D3D12_RESOURCE_FLAGS                   m_resourceFlags;

		bool                                   m_mapped {};

	private:
		[[nodiscard]] D3D12_RESOURCE_DESC DescribeBuffer() const;
	};

	class UploadBuffer final : public GpuBuffer
	{
	public:
		UploadBuffer() = default;
		DELETE_COPY_CTOR(UploadBuffer)
		DEFAULT_MOVE_CTOR(UploadBuffer)

		void Create(const std::wstring& name, size_t size);

		void* Map();
		void Unmap() const;

		[[nodiscard]] uint8_t* GetMappedData() const { return static_cast<uint8_t *>(m_memory); }

	private:
		void CreateViews() override {}
		void* m_memory {};
	};


	class IndexBuffer final : public GpuBuffer
	{
	public:
		IndexBuffer() = default;
		DELETE_COPY_CTOR(IndexBuffer)
		DEFAULT_MOVE_CTOR(IndexBuffer)

		~IndexBuffer() override = default;
		[[nodiscard]] const D3D12_INDEX_BUFFER_VIEW* GetIBV() const { return &m_ibv; }

	private:
		void CreateViews() override;

		D3D12_INDEX_BUFFER_VIEW m_ibv {};
	};


	class VertexBuffer final : public GpuBuffer
	{
	public:
		VertexBuffer() = default;
		DELETE_COPY_CTOR(VertexBuffer)
		DEFAULT_MOVE_CTOR(VertexBuffer)

		~VertexBuffer() override = default;
		[[nodiscard]] const D3D12_VERTEX_BUFFER_VIEW* GetVBV() const { return &m_vbv; }

	private:
		void CreateViews() override;
		D3D12_VERTEX_BUFFER_VIEW m_vbv {};
	};

	class ConstantBuffer final : public GpuBuffer
	{
	public:
		ConstantBuffer() = default;
		DELETE_COPY_CTOR(ConstantBuffer)
		DEFAULT_MOVE_CTOR(ConstantBuffer)

		void Destroy() override;

		[[nodiscard]] const DescriptorHandle& GetCBV() const { return m_cbvHandle; }

	private:
		void CreateViews() override;
		DescriptorHandle m_cbvHandle    {};
	};


	class StructuredBuffer final : public GpuBuffer
	{
	public:
		StructuredBuffer() { m_resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; }
		DELETE_COPY_CTOR(StructuredBuffer)
		DEFAULT_MOVE_CTOR(StructuredBuffer)
	private:
		void CreateViews() override;
	};
}
