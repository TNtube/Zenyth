#include "pch.hpp"

#include "Minicraft.hpp"

#include <DirectXColors.h>

#include "Core.hpp"
#include "Keyboard.h"
#include "Mouse.h"
#include "Win32Application.hpp"

#include "imgui.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;


Minicraft::Minicraft(const uint32_t width, const uint32_t height, const bool useWrapDevice)
	:	Zenyth::Application(width, height, useWrapDevice),
		m_aspectRatio(static_cast<float>(width) / static_cast<float>(height)),
		m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
		m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
		m_frameIndex(0),
		m_camera(XMConvertToRadians(80), m_aspectRatio)
{
	m_camera.SetPosition({0, 100, 10});
	m_world = std::make_unique<World>();
}

void Minicraft::OnInit()
{
	// Create input devices
	m_keyboard = std::make_unique<Keyboard>();
	m_mouse = std::make_unique<Mouse>();
	m_mouse->SetWindow(Zenyth::Win32Application::GetHwnd());
	LoadPipeline();
	LoadAssets();
}

void Minicraft::Tick() {
	m_timer.Tick([&]() {
		OnUpdate();
	});


	OnRender();
}

void Minicraft::OnUpdate()
{
	auto const kb = m_keyboard->GetState();
	m_camera.Update(static_cast<float>(m_timer.GetElapsedSeconds()), kb, m_mouse.get());
	const auto cameraData = m_camera.GetCameraData();
	m_cameraConstantBuffer[m_frameIndex]->SetData(cameraData);
}

void Minicraft::OnRender()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
		return;

	m_imguiLayer->Begin();

	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_imguiLayer->End();

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(0, 0));

	MoveToNextFrame();
}

void Minicraft::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForGpu();

	CloseHandle(m_fenceEvent);
}

void Minicraft::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	{
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug))))
		{
			m_debug->EnableDebugLayer();
		}

		// Enable additional debug layers.
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	ComPtr<IDXGIFactory4> factory;

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)), "Failed to create DXGIFactory");

	if (m_useWarpDevice)
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

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)), "Failed to create command queue");

	// Describe and create the swap chain.

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = GetWidth();
	swapChainDesc.Height = GetHeight();
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;

	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),// Swap chain needs the queue so that it can force a flush on it.
		Zenyth::Win32Application::GetHwnd(),
		&swapChainDesc,
		nullptr, nullptr,
		&swapChain), "Failed to create swap chain");

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(Zenyth::Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER), "Failed to make window association");

	ThrowIfFailed(swapChain.As(&m_swapChain), "Failed to get swap chain");
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		m_rtvHeap = std::make_unique<Zenyth::DescriptorHeap>();
		m_dsvHeap = std::make_unique<Zenyth::DescriptorHeap>();
		m_resourceHeap = std::make_unique<Zenyth::DescriptorHeap>();
		m_rtvHeap->Create(m_device.Get(), L"RTV Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FrameCount);
		m_dsvHeap->Create(m_device.Get(), L"DSV Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_DSV, FrameCount);
		m_resourceHeap->Create(m_device.Get(), L"Resource Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2048);
	}

	// Create frame resources.
	{
		// Create a RTV and a command allocator for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])), "Failed to get buffer");
			auto rtvHandle = m_rtvHeap->Alloc();
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle.CPU());

			ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])), "Failed to create command allocator");

			m_depthStencilBuffers[n] = std::make_unique<Zenyth::DepthStencilBuffer>();
			m_depthStencilBuffers[n]->Create(m_device.Get(), std::format(L"DepthStencilBuffer #{}", n), *m_dsvHeap, GetWidth(), GetHeight());
		}
	}

	// load imgui
	{
		m_imguiLayer = std::make_unique<Zenyth::ImGuiLayer>(m_device.Get(), m_commandQueue.Get());
	}

}

void Minicraft::LoadAssets()
{
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[3]; // Increase size to 3
		CD3DX12_ROOT_PARAMETER1 rootParameters[3]; // Increase size to 3

		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_VERTEX);

		// Create a static sampler for the texture
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		std::wstring vertexShaderPath = GetAssetFullPath(L"shaders/basic_vs.hlsl");
		std::wstring pixelShaderPath = GetAssetFullPath(L"shaders/basic_ps.hlsl");

//		ThrowIfFailed(D3DCompileFromFile(vertexShaderPath.c_str(), nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
//		ThrowIfFailed(D3DCompileFromFile(pixelShaderPath.c_str(), nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

		ComPtr<ID3DBlob> error;

		HRESULT hr;

		hr = D3DCompileFromFile(vertexShaderPath.c_str(), nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &vertexShader, &error);
		if (FAILED(hr))
		{
			if (error)
			{
				std::string output = static_cast<char *>(error->GetBufferPointer());
				std::cout << output << std::endl;
			}
			ThrowIfFailed(hr);
		}

		hr = D3DCompileFromFile(pixelShaderPath.c_str(), nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &pixelShader, &error);

		if (FAILED(hr))
		{
			if (error)
			{
				std::string output = static_cast<char *>(error->GetBufferPointer());
				std::cout << output << std::endl;
			}
			ThrowIfFailed(hr);
		}

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

		CD3DX12_DEPTH_STENCIL_DESC1 depthStencilDesc(D3D12_DEFAULT);

		D3D12_FEATURE_DATA_D3D12_OPTIONS2 featureOption = {};
		m_depthBoundsTestSupported = SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &featureOption, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS2))) && featureOption.DepthBoundsTestSupported;

		depthStencilDesc.DepthBoundsTestEnable = m_depthBoundsTestSupported;

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = depthStencilDesc;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	// Create the command list.
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

	// Create the chunk
	{

		m_world->Generate(m_device.Get(), *m_resourceHeap);
	}

	// Note: ComPtr's are CPU objects but this resource needs to stay in scope until
	// the command list that references it has finished executing on the GPU.
	// We will flush the GPU at the end of this method to ensure the resource is not
	// prematurely destroyed.
	ComPtr<ID3D12Resource> textureUploadHeap;

	// Create the texture.
	m_texture = Zenyth::Texture::LoadTextureFromFile(GetAssetFullPath(L"textures/terrain.dds").c_str(), m_device.Get(), m_commandQueue.Get(), *m_resourceHeap, m_commandList.Get());
	for (int n = 0; n < FrameCount; n++)
	{
		m_cameraConstantBuffer[n]= std::make_unique<Zenyth::ConstantBuffer<Zenyth::CameraData>>();
		m_cameraConstantBuffer[n]->Create(m_device.Get(), std::format(L"Camera Constant Buffer #{}", n), *m_resourceHeap);
	}

	// Close the command list and execute it to begin the initial GPU setup.
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		ThrowIfFailed(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		SUCCEEDED(m_fence->SetName(L"Minicraft Fence"));
		m_fenceValues[m_frameIndex]++;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command
		// list in our main loop but for now, we just want to wait for setup to
		// complete before continuing.
		WaitForGpu();
	}

}

void Minicraft::PopulateCommandList() const
{
	// Command list allocators can only be reset when the associated
	// command lists have finished execution on the GPU; apps should use
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

	// However, when ExecuteCommandList() is called on a particular command
	// list, that command list can then be reset at any time and must be before
	// re-recording.
	ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get()));

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_resourceHeap->GetHeapPointer() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);


	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &resourceBarrier);

	const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetStartHandle().CPU(), static_cast<INT>(m_frameIndex), m_rtvHeap->GetDescriptorSize());
	const auto dsvHandle = m_depthStencilBuffers[m_frameIndex]->GetDescriptorHandle().CPU();
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// Record commands.
	m_commandList->ClearRenderTargetView(rtvHandle, Colors::CornflowerBlue, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// apply cbv
	m_texture->Apply(m_commandList.Get(), 0);
	m_commandList->SetGraphicsRootDescriptorTable(2, m_cameraConstantBuffer[m_frameIndex]->GetDescriptorHandle().GPU());

	m_world->Draw(m_commandList.Get(), ShaderPass::Opaque);

	ImGui::ShowDemoWindow();
	m_imguiLayer->Render(m_commandList.Get());


	// Indicate that the back buffer will now be used to present.
	resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &resourceBarrier);

	ThrowIfFailed(m_commandList->Close());
}

// Wait for pending GPU work to complete.
void Minicraft::WaitForGpu()
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

	// Wait until the fence has been processed.
	ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_fenceValues[m_frameIndex]++;
}

void Minicraft::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

	// Update the frame index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

std::wstring Minicraft::GetAssetFullPath(const std::wstring &assetName)
{
	return L"resources/" + assetName;
}

void Minicraft::OnWindowSizeChanged(const int width, const int height)
{
	if (width == 0 || height == 0)
		return;

	m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);


	m_viewport.Width = static_cast<float>(width);
	m_viewport.Height = static_cast<float>(height);

	m_scissorRect.right = static_cast<LONG>(width);
	m_scissorRect.bottom = static_cast<LONG>(height);

	// Determine if the swap buffers and other resources need to be resized or not.
	if ((width != m_width || height != m_height))
	{
		m_width = width;
		m_height = height;
		// Flush all current GPU commands.
		WaitForGpu();

		// Release the resources holding references to the swap chain (requirement of
		// IDXGISwapChain::ResizeBuffers) and reset the frame fence values to the
		// current fence value.
		for (UINT n = 0; n < FrameCount; n++)
		{
			m_renderTargets[n].Reset();
			m_fenceValues[n] = m_fenceValues[m_frameIndex];
		}

		// Resize the swap chain to the desired dimensions.
		DXGI_SWAP_CHAIN_DESC desc = {};
		m_swapChain->GetDesc(&desc);
		ThrowIfFailed(m_swapChain->ResizeBuffers(FrameCount, width, height, desc.BufferDesc.Format, desc.Flags));

		BOOL fullscreenState;
		ThrowIfFailed(m_swapChain->GetFullscreenState(&fullscreenState, nullptr));

		// Reset the frame index to the current back buffer index.
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
		LoadSizeDependentResources();
	}
	// LoadAssets();
}


void Minicraft::LoadSizeDependentResources()
{
	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetStartHandle().CPU());
		m_dsvHeap->Destroy();
		m_dsvHeap->Create(m_device.Get(), L"DSV Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_DSV, FrameCount);

		// Create a RTV for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvHeap->GetDescriptorSize());

			m_depthStencilBuffers[n] = std::make_unique<Zenyth::DepthStencilBuffer>();
			m_depthStencilBuffers[n]->Create(m_device.Get(), std::format(L"DepthStencilBuffer #{}", n), *m_dsvHeap, m_width, m_height);
		}
	}
}
