#pragma once


namespace Zenyth
{

	class GpuResource
	{
		friend class CommandBatch;

	public:
		GpuResource() :
			m_UsageState(D3D12_RESOURCE_STATE_COMMON),
			m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		{
		}

		virtual ~GpuResource() { GpuResource::Destroy(); }

		virtual void Destroy();

		ID3D12Resource* operator->() { return m_resource.Get(); }
		const ID3D12Resource* operator->() const { return m_resource.Get(); }

		ID3D12Resource* GetResource() { return m_resource.Get(); }
		[[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() const { return m_resource; }
		[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_GpuVirtualAddress; }
		virtual ID3D12Resource** GetAddressOf() { return m_resource.GetAddressOf(); }

		[[nodiscard]] bool CheckFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport ) const;
		[[nodiscard]] bool CheckFormatSupport(D3D12_FORMAT_SUPPORT2 formatSupport) const;

	protected:
		virtual void Create(const Microsoft::WRL::ComPtr<ID3D12Resource>& resource, const D3D12_CLEAR_VALUE* clearValue);
		virtual void Create(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue);
		void CheckFeatureSupport();

		Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
		D3D12_RESOURCE_STATES				  m_UsageState;
		D3D12_GPU_VIRTUAL_ADDRESS			  m_GpuVirtualAddress;
		D3D12_FEATURE_DATA_FORMAT_SUPPORT	  m_FormatSupport {};
		std::unique_ptr<D3D12_CLEAR_VALUE> m_clearValue;
	};
}
