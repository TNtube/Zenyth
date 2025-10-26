#include "pch.hpp"

#include "Renderer/GpuResource.hpp"

#include "Application.hpp"

namespace Zenyth
{
	void GpuResource::Create(const Microsoft::WRL::ComPtr<ID3D12Resource>& resource,
		const D3D12_CLEAR_VALUE* clearValue)
	{
		m_resource = resource;

		if (clearValue)
			m_clearValue = std::make_unique<D3D12_CLEAR_VALUE>( *clearValue );

		CheckFeatureSupport();
	}

	void GpuResource::Create(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue)
	{
		GpuResource::Destroy();

		const CD3DX12_HEAP_PROPERTIES heapDesc( D3D12_HEAP_TYPE_DEFAULT );
		const auto device = Application::Get().GetRenderer().GetDevice();

		ThrowIfFailed( device->CreateCommittedResource(
			&heapDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc,
			m_UsageState, clearValue, IID_PPV_ARGS( m_resource.ReleaseAndGetAddressOf() ) ) );

		if (clearValue)
			m_clearValue = std::make_unique<D3D12_CLEAR_VALUE>( *clearValue );

		CheckFeatureSupport();
	}

	void GpuResource::Destroy()
	{
		m_resource = nullptr;
		m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
	}

	bool GpuResource::CheckFormatSupport(const D3D12_FORMAT_SUPPORT1 formatSupport) const
	{
		return ( m_FormatSupport.Support1 & formatSupport ) != 0;
	}

	bool GpuResource::CheckFormatSupport(const D3D12_FORMAT_SUPPORT2 formatSupport) const
	{
		return ( m_FormatSupport.Support2 & formatSupport ) != 0;
	}

	void GpuResource::CheckFeatureSupport()
	{
		const auto device = Application::Get().GetRenderer().GetDevice();
		m_FormatSupport.Format = m_resource->GetDesc().Format;
		ThrowIfFailed(device->CheckFeatureSupport(
			D3D12_FEATURE_FORMAT_SUPPORT, &m_FormatSupport,
			sizeof( D3D12_FEATURE_DATA_FORMAT_SUPPORT ) ));
	}
}
