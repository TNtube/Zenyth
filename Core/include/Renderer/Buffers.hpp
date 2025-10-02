#pragma once
#include "DescriptorHeap.hpp"
#include "Core.hpp"

namespace Zenyth {
	class GpuBuffer
	{
		friend class CommandBatch;
	public:
		GpuBuffer() = default;

		DELETE_COPY_CTOR(GpuBuffer)
		DEFAULT_MOVE_CTOR(GpuBuffer)

		virtual ~GpuBuffer();

		void Create(ID3D12Device *device, const std::wstring& name, size_t numElements, size_t elementSize, bool align = true, const void* initialData = nullptr);
		void Destroy() { m_buffer.Reset(); }

		[[nodiscard]] ID3D12Resource* Get() const { return m_buffer.Get(); }
		ID3D12Resource** GetAddressOf() { return m_buffer.GetAddressOf(); }

		[[nodiscard]] virtual bool IsValid() const { return m_buffer != nullptr; }
		[[nodiscard]] uint32_t GetElementCount() const { return m_elementCount; }
		[[nodiscard]] uint32_t GetElementSize() const { return m_elementSize; }
		[[nodiscard]] size_t GetBufferSize() const { return m_bufferSize; }

	protected:
		virtual void CreateViews() = 0;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer {};
		ID3D12Device*                          m_pDevice {};
		size_t                                 m_bufferSize {};
		uint32_t                               m_elementCount {};
		uint32_t                               m_elementSize {};
		D3D12_RESOURCE_STATES                  m_resourceState {D3D12_RESOURCE_STATE_COMMON};
		bool                                   m_mapped {};
	};

	class UploadBuffer final : public GpuBuffer
	{
	public:
		UploadBuffer() = default;

		DELETE_COPY_CTOR(UploadBuffer)
		DEFAULT_MOVE_CTOR(UploadBuffer)

		void Create(const std::wstring& name, size_t size);

		void *Map();
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
		explicit ConstantBuffer(DescriptorHeap& resourceHeap) : m_resourceHeap(&resourceHeap) {};

		DELETE_COPY_CTOR(ConstantBuffer)
		DEFAULT_MOVE_CTOR(ConstantBuffer)

		~ConstantBuffer() override;

		[[nodiscard]] const DescriptorHandle& GetCBV() const { return m_cbvHandle; }

	private:
		void CreateViews() override;

		DescriptorHeap*  m_resourceHeap {};
		DescriptorHandle m_cbvHandle    {};
	};


	class StructuredBuffer final : public GpuBuffer
	{
	public:
		explicit StructuredBuffer(DescriptorHeap &resourceHeap) : m_resourceHeap(&resourceHeap) {};

		DELETE_COPY_CTOR(StructuredBuffer)
		DEFAULT_MOVE_CTOR(StructuredBuffer)

		~StructuredBuffer() override;

		[[nodiscard]] const DescriptorHandle& GetSrv() const { return m_srvHandle; }
	private:
		void CreateViews() override;

		DescriptorHeap*  m_resourceHeap {};
		DescriptorHandle m_srvHandle    {};

	};
}
