#include "pch.hpp"

#include "Minicraft.hpp"

#include <DirectXColors.h>

#include "Core.hpp"
#include "Keyboard.h"
#include "Mouse.h"
#include "Win32Application.hpp"

#include "imgui.h"
#include "Renderer/CommandBatch.hpp"
#include "Renderer/Renderer.hpp"

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
	const auto cameraData = m_camera.GetCameraData(m_timer);
	// m_cameraConstantBuffer->SetData(cameraData, m_frameIndex);
	memcpy(m_cameraCpuBuffer->GetMappedData(), &cameraData, sizeof(Zenyth::CameraData));
}

void Minicraft::OnRender()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
		return;

	m_imguiLayer->Begin();

	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	m_imguiLayer->End();

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(0, 0));

	MoveToNextFrame();
}

void Minicraft::OnDestroy()
{
	Zenyth::Renderer::Shutdown();
}

void Minicraft::LoadPipeline()
{
	Zenyth::Renderer::Initialize(m_useWarpDevice);

	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)), "Failed to create DXGIFactory");

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
		Zenyth::Renderer::pCommandManager->GetGraphicsQueue().GetCommandQueue(),// Swap chain needs the queue so that it can force a flush on it.
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
		m_rtvHeap->Create(Zenyth::Renderer::pDevice.Get(), L"RTV Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FrameCount);
		m_dsvHeap->Create(Zenyth::Renderer::pDevice.Get(), L"DSV Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_DSV, FrameCount);
		m_resourceHeap->Create(Zenyth::Renderer::pDevice.Get(), L"Resource Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2048);
	}

	// Create frame resources.
	{
		// Create a RTV and a command allocator for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ID3D12Resource* backBuffer = nullptr;
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&backBuffer)), "Failed to get buffer");

			m_renderTargets[n] = std::make_unique<Zenyth::ColorBuffer>(*m_rtvHeap, *m_resourceHeap);
			m_renderTargets[n]->CreateFromSwapChain(std::format(L"Render Target #{}", n), backBuffer);

			m_depthStencilBuffers[n] = std::make_unique<Zenyth::DepthStencilBuffer>(*m_dsvHeap);
			m_depthStencilBuffers[n]->Create(Zenyth::Renderer::pDevice.Get(), std::format(L"DepthStencilBuffer #{}", n), GetWidth(), GetHeight());
		}
	}

	// load imgui
	{
		m_imguiLayer = std::make_unique<Zenyth::ImGuiLayer>(Zenyth::Renderer::pDevice.Get(), Zenyth::Renderer::pCommandManager->GetGraphicsQueue().GetCommandQueue());
	}

}

void Minicraft::LoadAssets()
{
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(Zenyth::Renderer::pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
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
		ThrowIfFailed(Zenyth::Renderer::pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

#ifndef NDEBUG
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
		m_depthBoundsTestSupported = SUCCEEDED(Zenyth::Renderer::pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &featureOption, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS2))) && featureOption.DepthBoundsTestSupported;

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
		ThrowIfFailed(Zenyth::Renderer::pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	// Create the chunk
	m_world->Generate(Zenyth::Renderer::pDevice.Get(), *m_resourceHeap);

	m_cameraCpuBuffer = std::make_unique<Zenyth::UploadBuffer>();
	m_cameraCpuBuffer->Create(L"Camera Upload Buffer", sizeof(Zenyth::CameraData));
	m_cameraCpuBuffer->Map();
	m_cameraConstantBuffer = std::make_unique<Zenyth::ConstantBuffer>(*m_resourceHeap);
	m_cameraConstantBuffer->Create(Zenyth::Renderer::pDevice.Get(), L"Camera Constant Buffer", 3, (sizeof(Zenyth::CameraData) + 255) & ~255);

	auto& commandManager = *Zenyth::Renderer::pCommandManager;

	// Create the texture.
	m_texture = Zenyth::Texture::LoadTextureFromFile(GetAssetFullPath(L"textures/terrain.dds").c_str(), *m_resourceHeap);

	commandManager.IdleGPU();
}

void Minicraft::PopulateCommandList()
{
	auto commandBatch = Zenyth::CommandBatch::Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto* commandList = commandBatch.GetCommandList();

	commandList->SetPipelineState(m_pipelineState.Get());

	// Set necessary state.
	commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_resourceHeap->GetHeapPointer() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);


	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);

	commandBatch.TransitionResource(*m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	const auto dsvHandle = m_depthStencilBuffers[m_frameIndex]->GetDSV().CPU();
	const auto rtvHandle = m_renderTargets[m_frameIndex]->GetRTV().CPU();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// Record commands.
	commandList->ClearRenderTargetView(rtvHandle, Colors::MidnightBlue, 0, nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandBatch.CopyBufferRegion(*m_cameraConstantBuffer, m_frameIndex * m_cameraConstantBuffer->GetElementSize(), *m_cameraCpuBuffer, 0, sizeof(Zenyth::CameraData));
	commandBatch.TransitionResource(*m_cameraConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, true);

	// apply cbv
	commandList->SetGraphicsRootDescriptorTable(0, m_texture->GetSRV().GPU());
	commandList->SetGraphicsRootDescriptorTable(2, m_cameraConstantBuffer->GetCBV().GPU());

	m_world->Draw(commandList, ShaderPass::Opaque);

	ImGui::ShowDemoWindow();
	ImGui::Begin("FPS");
	ImGui::Text("Average FPS: %.2f", 1.0f / m_timer.GetElapsedSeconds());
	ImGui::End();
	m_imguiLayer->Render(commandList);


	// Indicate that the back buffer will now be used to present.
	commandBatch.TransitionResource(*m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT, true);

	const auto fence = commandBatch.End();
	m_fenceValues[m_frameIndex] = fence;
}

void Minicraft::MoveToNextFrame()
{
	auto& graphicsQueue = Zenyth::Renderer::pCommandManager->GetGraphicsQueue();

	// Update the frame index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	graphicsQueue.WaitForFence(m_fenceValues[m_frameIndex]);
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
	m_camera.UpdateAspectRatio(m_aspectRatio);


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
		Zenyth::Renderer::pCommandManager->IdleGPU();

		// Release the resources holding references to the swap chain (requirement of
		// IDXGISwapChain::ResizeBuffers) and reset the frame fence values to the
		// current fence value.
		for (UINT n = 0; n < FrameCount; n++)
		{
			m_renderTargets[n]->Destroy();
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


void Minicraft::LoadSizeDependentResources() const
{
	// Create frame resources.
	{
		// Create a RTV for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ID3D12Resource* backBuffer = nullptr;
			SUCCEEDED(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&backBuffer)));
			m_renderTargets[n]->CreateFromSwapChain(std::format(L"Render Target #{}", n), backBuffer);

			m_depthStencilBuffers[n]->Create(Zenyth::Renderer::pDevice.Get(), std::format(L"DepthStencilBuffer #{}", n), m_width, m_height);
		}
	}
}
