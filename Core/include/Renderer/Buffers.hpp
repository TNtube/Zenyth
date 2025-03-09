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

		void Create(ID3D12Device *device, const std::wstring& name, uint32_t numElements, uint32_t elementSize, const void* initialData = nullptr);
		void Destroy() { m_buffer.Reset(); }

		ID3D12Resource* Get() const { return m_buffer.Get(); }
		ID3D12Resource** GetAddressOf() { return m_buffer.GetAddressOf(); }

		void Map(UINT8** pDataBegin);
		void Unmap();

		[[nodiscard]] virtual bool IsValid() const { return m_buffer != nullptr; }
		[[nodiscard]] uint32_t GetElementCount() const { return m_elementCount; }

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

	class ColorBuffer final : public GpuBuffer
	{
	public:
		explicit ColorBuffer(DescriptorHeap& rtvHeap, DescriptorHeap& resourceHeap) : m_rtvHeap(&rtvHeap), m_resourceHeap(&resourceHeap) {};

		DELETE_COPY_CTOR(ColorBuffer)
		DEFAULT_MOVE_CTOR(ColorBuffer)

		~ColorBuffer() override = default;

		void CreateFromSwapChain(const std::wstring& name, ID3D12Resource* swapChainBuffer);
		void Create(const std::wstring& name, uint32_t width, uint32_t height);

		[[nodiscard]] const DescriptorHandle& GetRTV() const { return m_rtvHandle; }
		[[nodiscard]] const DescriptorHandle& GetSRV() const { return m_srvHandle; }
		void CreateViews() override;

	private:
		DescriptorHeap* m_rtvHeap;
		DescriptorHeap* m_resourceHeap;

		DescriptorHandle m_rtvHandle {};
		DescriptorHandle m_srvHandle {};
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
		[[nodiscard]] uint8_t* MappedData() const { return m_mappedData; }

	private:
		void CreateViews() override;

		DescriptorHeap*  m_resourceHeap {};
		DescriptorHandle m_cbvHandle    {};
		uint8_t*         m_mappedData   {};
	};

	class DepthStencilBuffer final : public GpuBuffer
	{
	public:
		explicit DepthStencilBuffer(DescriptorHeap& dsvHeap) { m_dsvHeap = &dsvHeap; }

		DELETE_COPY_CTOR(DepthStencilBuffer)
		DEFAULT_MOVE_CTOR(DepthStencilBuffer)

		~DepthStencilBuffer() override;

		// no need to re-instantiate the buffer for resizing it
		void Create(ID3D12Device *device, const std::wstring& name, uint32_t width, uint32_t height);

		[[nodiscard]] const DescriptorHandle& GetDSV() const { return m_dsvHandle; }
	private:
		void CreateViews() override;

		DescriptorHeap*  m_dsvHeap   {};
		DescriptorHandle m_dsvHandle {};
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
