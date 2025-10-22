#include "pch.hpp"
#include "Core.hpp"

#include "Renderer/Renderer.hpp"
#include <dxgidebug.h>

namespace Zenyth
{
	using Microsoft::WRL::ComPtr;

	Renderer::Renderer(const bool useWrapDevice)
	{
		UINT dxgiFactoryFlags = 0;

#ifndef NDEBUG
		{
			ComPtr<ID3D12Debug> debug;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
			{
				debug->EnableDebugLayer();
			}

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
#endif

		ComPtr<IDXGIFactory4> factory;

		ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)), "Failed to create DXGIFactory");

		if (useWrapDevice)
		{
			ComPtr<IDXGIAdapter> wrapAdapter;
			ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&wrapAdapter)), "Failed to create warp adapter");

			ThrowIfFailed(D3D12CreateDevice(wrapAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)), "Failed to create device");
		}
		else
		{
			ComPtr<IDXGIAdapter1> hardwareAdapter;
			GetHardwareAdapter(factory.Get(), &hardwareAdapter);

			ThrowIfFailed(D3D12CreateDevice(
				hardwareAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&m_device)
			));
		}
	}

	void Renderer::Init()
	{
		m_commandManager.Create(m_device.Get());

		m_descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Create(L"CBV SRV UAV Descriptor Heap", 4096);
		m_descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].Create(L"Sampler Descriptor Heap", 64);
		m_descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].Create(L"RTV Descriptor Heap", 64);
		m_descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].Create(L"DSV Descriptor Heap", FrameCount);
	}

	DescriptorHandle Renderer::AllocateDescriptor(const D3D12_DESCRIPTOR_HEAP_TYPE type, const int64_t count)
	{
		return m_descriptorHeaps[type].Alloc(count);
	}

	void Renderer::FreeDescriptor(const DescriptorHandle& handle, const int64_t count)
	{
		return m_descriptorHeaps[handle.GetType()].Free(handle, count);
	}

	void Renderer::GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, const bool requestHighPerformanceAdapter)
	{
		*ppAdapter = nullptr;

		ComPtr<IDXGIAdapter1> adapter;

		ComPtr<IDXGIFactory6> factory6;
		if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
		{
			for (
				UINT adapterIndex = 0;
				SUCCEEDED(factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
					IID_PPV_ARGS(&adapter)));
				++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;

				if (SUCCEEDED(adapter->GetDesc1(&desc)) && desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}

		if(adapter.Get() == nullptr)
		{
			for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;

				if (SUCCEEDED(adapter->GetDesc1(&desc)) && desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}

		*ppAdapter = adapter.Detach();
	}
}
