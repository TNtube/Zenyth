#include "pch.hpp"

#include "ModelGallery.hpp"

#include <DirectXColors.h>

#include "Core.hpp"
#include "Keyboard.h"
#include "Mouse.h"
#include "Win32Application.hpp"

#include "imgui.h"
#include "Data/Transform.hpp"
#include "Assets/ObjLoader.hpp"
#include "Renderer/CommandBatch.hpp"
#include "Renderer/PixelBuffer.hpp"
#include "Renderer/Renderer.hpp"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

enum class GBufferType
{
	Color,
	Normal
};


ModelGallery::ModelGallery(const uint32_t width, const uint32_t height, const bool useWrapDevice)
	:	Zenyth::Application(width, height, useWrapDevice),
		m_aspectRatio(static_cast<float>(width) / static_cast<float>(height)),
		m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
		m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
		m_frameIndex(0),
		m_camera(XMConvertToRadians(80), m_aspectRatio)
{
	m_camera.SetPosition({0, 0, 10});
}

void ModelGallery::OnInit()
{
	// Create input devices
	m_keyboard = std::make_unique<Keyboard>();
	m_mouse = std::make_unique<Mouse>();
	m_mouse->SetWindow(Zenyth::Win32Application::GetHwnd());
	LoadPipeline();
	LoadAssets();
}

void ModelGallery::Tick() {
	m_timer.Tick([&]() {
		OnUpdate();
	});


	OnRender();
}

void ModelGallery::OnUpdate()
{
	auto const kb = m_keyboard->GetState();
	m_camera.Update(static_cast<float>(m_timer.GetElapsedSeconds()), kb, m_mouse.get());
	const auto cameraData = m_camera.GetCameraData(m_timer);
	// m_cameraConstantBuffer->SetData(cameraData, m_frameIndex);
	memcpy(m_cameraCpuBuffer->GetMappedData(), &cameraData, sizeof(Zenyth::CameraData));
}

void ModelGallery::OnRender()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
		return;

	m_imguiLayer->Begin();

	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	m_imguiLayer->End();

	// Present the frame. vsync enabled
	ThrowIfFailed(m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING));

	MoveToNextFrame();
}

void ModelGallery::OnDestroy()
{
	Zenyth::Renderer::Shutdown();
}

struct ModelData
{
	Matrix model;
};
void ModelGallery::LoadPipeline()
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
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

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
		m_rtvHeap->Create(Zenyth::Renderer::pDevice.Get(), L"RTV Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 64);
		m_dsvHeap->Create(Zenyth::Renderer::pDevice.Get(), L"DSV Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_DSV, FrameCount);
		m_resourceHeap->Create(Zenyth::Renderer::pDevice.Get(), L"Resource Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2048); // :skull:
	}

	// Create frame resources.
	{
		// Create a RTV and a command allocator for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ID3D12Resource* backBuffer = nullptr;
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&backBuffer)), "Failed to get buffer");

			m_renderTargets[n] = std::make_unique<Zenyth::PixelBuffer>(*m_rtvHeap, *m_resourceHeap);
			m_renderTargets[n]->CreateFromSwapChain(std::format(L"Render Target #{}", n), backBuffer);
		}
	}

	// load imgui
	{
		m_imguiLayer = std::make_unique<Zenyth::ImGuiLayer>(Zenyth::Renderer::pDevice.Get(), Zenyth::Renderer::pCommandManager->GetGraphicsQueue().GetCommandQueue());
	}

	{
		m_depthStencilBuffer = std::make_unique<Zenyth::DepthStencilBuffer>(*m_dsvHeap, *m_resourceHeap);
		m_depthStencilBuffer->Create(Zenyth::Renderer::pDevice.Get(), L"DepthStencilBuffer", GetWidth(), GetHeight());
	}

	{
		Zenyth::ObjLoader doorObj(GetAssetFullPath(L"models/SM_Door/cube-tex.obj"));

		if (doorObj.LoadData())
		{
			m_meshes = doorObj.GenerateRenderers();
		}
	}

	{
		Zenyth::Transform transform;
		// transform.SetScale(30, 30, 30);
		m_meshConstantBuffer = std::make_unique<Zenyth::ConstantBuffer>(*m_resourceHeap);
		const ModelData modelData = {transform.GetTransformMatrix()};
		m_meshConstantBuffer->Create(Zenyth::Renderer::pDevice.Get(), L"Model Constant Buffer", 1, Zenyth::Math::AlignToMask(static_cast<int>(sizeof(ModelData)), 256), &modelData);
	}
}

void ModelGallery::LoadAssets()
{
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS2 featureOption = {};
		m_depthBoundsTestSupported = SUCCEEDED(Zenyth::Renderer::pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &featureOption, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS2))) && featureOption.DepthBoundsTestSupported;

		m_pipelineGeometry = std::make_unique<Zenyth::Pipeline>();
		m_pipelineGeometry->Create(L"Geometry Pipeline", GetAssetFullPath(L"shaders/basic_vs.hlsl"), GetAssetFullPath(L"shaders/basic_ps.hlsl"), m_depthBoundsTestSupported);
	}

	m_cameraCpuBuffer = std::make_unique<Zenyth::UploadBuffer>();
	m_cameraCpuBuffer->Create(L"Camera Upload Buffer", sizeof(Zenyth::CameraData));
	m_cameraCpuBuffer->Map();
	m_cameraConstantBuffer = std::make_unique<Zenyth::ConstantBuffer>(*m_resourceHeap);
	m_cameraConstantBuffer->Create(Zenyth::Renderer::pDevice.Get(), L"Camera Constant Buffer", 3, (sizeof(Zenyth::CameraData) + 255) & ~255);

	auto& commandManager = *Zenyth::Renderer::pCommandManager;

	// Create the texture.
	m_tileset = Zenyth::Texture::LoadTextureFromFile(GetAssetFullPath(L"textures/texture.dds").c_str(), *m_resourceHeap);
	m_tilesetNormal = Zenyth::Texture::LoadTextureFromFile(GetAssetFullPath(L"textures/terrain_n.dds").c_str(), *m_resourceHeap);

	commandManager.IdleGPU();
}

void ModelGallery::PopulateCommandList()
{

	auto commandBatch = Zenyth::CommandBatch::Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto* commandList = commandBatch.GetCommandList();


	commandList->SetPipelineState(m_pipelineGeometry->Get());

	commandList->SetGraphicsRootSignature(m_pipelineGeometry->GetRootSignature());

	ID3D12DescriptorHeap* ppHeaps[] = { m_resourceHeap->GetHeapPointer() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);


	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);

	commandBatch.TransitionResource(*m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	commandBatch.TransitionResource(*m_depthStencilBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);


	const auto dsvHandle = m_depthStencilBuffer->GetDSV().CPU();

	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);

	const auto rtvHandle = m_renderTargets[m_frameIndex]->GetRTV().CPU();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	commandList->ClearRenderTargetView(rtvHandle, Colors::CornflowerBlue, 0, nullptr);


	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);

	// Record commands.
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandBatch.CopyBufferRegion(*m_cameraConstantBuffer, m_frameIndex * m_cameraConstantBuffer->GetElementSize(), *m_cameraCpuBuffer, 0, sizeof(Zenyth::CameraData));
	commandBatch.TransitionResource(*m_cameraConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, true);

	// apply cbv
	commandList->SetGraphicsRootDescriptorTable(m_pipelineGeometry->GetRootParameterIndex("AlbedoTexture"), m_tileset->GetSRV().GPU());
	// commandList->SetGraphicsRootDescriptorTable(m_pipelineGeometry->GetRootParameterIndex("NormalTexture"), m_tilesetNormal->GetSRV().GPU());
	commandList->SetGraphicsRootDescriptorTable(m_pipelineGeometry->GetRootParameterIndex("CameraData"), m_cameraConstantBuffer->GetCBV().GPU());


	commandList->SetGraphicsRootDescriptorTable(0, m_meshConstantBuffer->GetCBV().GPU());
	for (const auto& mesh : m_meshes)
	{
		mesh.Draw(commandList);
	}


	ImGui::Begin("Helper");
	ImGui::Text("FPS: %.2f", 1.0f / m_timer.GetElapsedSeconds());

	// m_camera.OnImGui();


	ImGui::End();
	m_imguiLayer->Render(commandList);

	commandBatch.TransitionResource(*m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT, true);


	const auto fence = commandBatch.End();
	m_fenceValues[m_frameIndex] = fence;
}

void ModelGallery::MoveToNextFrame()
{
	auto& graphicsQueue = Zenyth::Renderer::pCommandManager->GetGraphicsQueue();

	// Update the frame index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	graphicsQueue.WaitForFence(m_fenceValues[m_frameIndex]);
}

std::wstring ModelGallery::GetAssetFullPath(const std::wstring& assetName)
{
	return WORKING_DIR L"resources/" + assetName;
}

void ModelGallery::OnWindowSizeChanged(const int width, const int height)
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
}


void ModelGallery::LoadSizeDependentResources() const
{
	// Create frame resources.
	{
		// Create a RTV for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ID3D12Resource* backBuffer = nullptr;
			SUCCEEDED(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&backBuffer)));
			m_renderTargets[n]->CreateFromSwapChain(std::format(L"Render Target #{}", n), backBuffer);
		}

		m_depthStencilBuffer->Create(Zenyth::Renderer::pDevice.Get(), L"DepthStencilBuffer", m_width, m_height);
	}
}
