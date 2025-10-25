#include "pch.hpp"

#include "ModelGallery.hpp"

#include <DirectXColors.h>

#include "Core.hpp"
#include "Keyboard.h"
#include "Mouse.h"
#include "Win32Application.hpp"

#include "imgui.h"
#include "Data/Transform.hpp"
#include "Data/Light.hpp"
#include "Data/Mesh.hpp"
#include "Renderer/CommandBatch.hpp"
#include "Renderer/Renderer.hpp"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;


ModelGallery::ModelGallery(const uint32_t width, const uint32_t height, const bool useWrapDevice)
	:	Zenyth::Application(width, height, useWrapDevice),
		m_aspectRatio(static_cast<float>(width) / static_cast<float>(height)),
		m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
		m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
		m_frameIndex(0),
		m_camera(XMConvertToRadians(80), m_aspectRatio)
{
	m_camera.SetPosition({0, 1, 3});

	m_directionalLight.direction = Vector3{0.3, -1.0, 0.2};
	m_directionalLight.type = Zenyth::LightType::Directional;
	m_directionalLight.color = Colors::LightYellow;

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
	const auto dt = static_cast<float>(m_timer.GetElapsedSeconds());
	auto const kb = m_keyboard->GetState();
	m_camera.Update(dt, kb, m_mouse.get());
	const auto cameraData = m_camera.GetCameraData();
	memcpy(m_cameraCpuBuffer->GetMappedData(), &cameraData, sizeof(Zenyth::CameraData));

	memcpy(m_lightUploadBuffer->GetMappedData(), &m_directionalLight, sizeof(Zenyth::LightData));

	SceneConstants sc;

	sc.activeLightCount = 1;
	sc.cameraPosition = m_camera.GetPosition();
	sc.time = m_timer.GetTotalSeconds();
	sc.deltaTime = m_timer.GetElapsedSeconds();
	sc.screenResolution = Vector2(m_width, m_height);

	memcpy(m_sceneUploadBuffer->GetMappedData(), &sc, sizeof(SceneConstants));
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
	GetRenderer().GetCommandManager().IdleGPU();
}

struct ModelData
{
	Matrix model;
};

void ModelGallery::LoadPipeline()
{
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
		GetRenderer().GetCommandManager().GetGraphicsQueue().GetCommandQueue(),// Swap chain needs the queue so that it can force a flush on it.
		Zenyth::Win32Application::GetHwnd(),
		&swapChainDesc,
		nullptr, nullptr,
		&swapChain), "Failed to create swap chain");

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(Zenyth::Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER), "Failed to make window association");

	ThrowIfFailed(swapChain.As(&m_swapChain), "Failed to get swap chain");
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	{
		for (UINT n = 0; n < FrameCount; n++)
		{
			const auto& renderTarget = m_renderTargets[n] = std::make_unique<Zenyth::RenderTarget>();

			auto backBufferTexture = std::make_unique<Zenyth::Texture>();
			auto depthStencilTexture = std::make_unique<Zenyth::Texture>();

			renderTarget->AttachTexture(Zenyth::AttachmentPoint::Color0, std::move(backBufferTexture));
			renderTarget->AttachTexture(Zenyth::AttachmentPoint::DepthStencil, std::move(depthStencilTexture));
		}
	}

	LoadSizeDependentResources();

	// load imgui
	{
		m_imguiLayer = std::make_unique<Zenyth::ImGuiLayer>(GetRenderer().GetDevice(), GetRenderer().GetCommandManager().GetGraphicsQueue().GetCommandQueue());
	}
}

void ModelGallery::LoadAssets()
{
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS2 featureOption = {};
		m_depthBoundsTestSupported = SUCCEEDED(GetRenderer().GetDevice()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &featureOption, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS2))) && featureOption.DepthBoundsTestSupported;

		m_pipelineGeometry = std::make_unique<Zenyth::Pipeline>();
		m_pipelineGeometry->Create(L"Geometry Pipeline", GetAssetFullPathW(L"shaders/basic_vs.hlsl"), GetAssetFullPathW(L"shaders/basic_ps.hlsl"), m_depthBoundsTestSupported);
	}

	{
		m_cameraCpuBuffer = std::make_unique<Zenyth::UploadBuffer>();
		m_cameraCpuBuffer->Create(L"Camera Upload Buffer", sizeof(Zenyth::CameraData));
		m_cameraCpuBuffer->Map();

		m_cameraConstantBuffer = std::make_unique<Zenyth::ConstantBuffer>();
		m_cameraConstantBuffer->Create(
			L"Camera Constant Buffer",
			3,
			sizeof(Zenyth::CameraData), nullptr, true);


		m_sceneUploadBuffer = std::make_unique<Zenyth::UploadBuffer>();
		m_sceneUploadBuffer->Create(L"Scene Upload Buffer", sizeof(SceneConstants));
		m_sceneUploadBuffer->Map();

		m_sceneConstantBuffer = std::make_unique<Zenyth::ConstantBuffer>();
		m_sceneConstantBuffer->Create(
			L"Scene Constant Buffer",
			3,
			sizeof(SceneConstants), nullptr, true);
	}

	{
		Zenyth::Transform transform;
		transform.SetEulerAngles(0, XMConvertToRadians(-180), 0);
		transform.SetScale(.1, .1, .1);
		const ModelData modelData = {transform.GetTransformMatrix()};

		m_meshConstantBuffer = std::make_unique<Zenyth::ConstantBuffer>();
		m_meshConstantBuffer->Create(
			L"Model Constant Buffer",
			1,
			sizeof(ModelData),
			&modelData, true);
	}

	{
		// Zenyth::ObjLoader doorObj(GetAssetFullPath(L"models/SM_Door/SM_Door.obj"));
		//
		// if (doorObj.LoadData())
		// {
		// 	m_meshes = doorObj.GenerateRenderers();
		// }

		Zenyth::Mesh mesh;
		if (Zenyth::Mesh::FromObjFile(GetAssetFullPath("models/sponza/sponza.obj"), mesh))
		{
			m_meshRenderer = std::make_unique<Zenyth::MeshRenderer>(mesh);
		}
	}


	{
		m_lightUploadBuffer = std::make_unique<Zenyth::UploadBuffer>();
		m_lightUploadBuffer->Create(L"Light Upload Buffer", sizeof(Zenyth::LightData));
		m_lightUploadBuffer->Map();

		m_lightBuffer = std::make_unique<Zenyth::StructuredBuffer>();
		m_lightBuffer->Create(
			L"Light Structured Buffer",
			3,
			sizeof(Zenyth::LightData));
	}

	auto& commandManager = GetRenderer().GetCommandManager();
	commandManager.IdleGPU();
	// Create the texture.
	// m_tileset = Zenyth::Texture::LoadTextureFromFile(GetAssetFullPath("models/SM_Door/T_Door_BC.png").c_str(), GetRenderer().GetResourceHeap());
	// m_tilesetNormal = Zenyth::Texture::LoadTextureFromFile(GetAssetFullPath("models/SM_Door/T_Door_N.png").c_str(), GetRenderer().GetResourceHeap(), false);
	// m_tilesetSpecular = Zenyth::Texture::LoadTextureFromFile(GetAssetFullPath("models/SM_Door/T_Door_R.png").c_str(), GetRenderer().GetResourceHeap());

}

void ModelGallery::PopulateCommandList()
{

	auto commandBatch = Zenyth::CommandBatch::Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto* commandList = commandBatch.GetCommandList();


	commandList->SetPipelineState(m_pipelineGeometry->Get());

	commandList->SetGraphicsRootSignature(m_pipelineGeometry->GetRootSignature());

	ID3D12DescriptorHeap* ppHeaps[] = { GetRenderer().GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetHeapPointer() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);


	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);

	const auto color0 = m_renderTargets[m_frameIndex]->GetTexture(Zenyth::AttachmentPoint::Color0);
	const auto depth = m_renderTargets[m_frameIndex]->GetTexture(Zenyth::AttachmentPoint::DepthStencil);
	commandBatch.TransitionBarrier(*color0, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	commandBatch.TransitionBarrier(*depth, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);


	const auto dsvHandle = depth->GetDSV();
	const auto rtvHandle = color0->GetRTV();

	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	commandList->ClearRenderTargetView(rtvHandle, Color{0.2f, 0.2f, 0.2f, 1.0f}, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);

	// Record commands.
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandBatch.CopyBufferRegion(*m_cameraConstantBuffer, m_frameIndex * m_cameraConstantBuffer->GetElementSize(), *m_cameraCpuBuffer, 0, sizeof(Zenyth::CameraData));
	commandBatch.TransitionBarrier(*m_cameraConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	commandBatch.CopyBufferRegion(*m_sceneConstantBuffer, m_frameIndex * m_sceneConstantBuffer->GetElementSize(), *m_sceneUploadBuffer, 0, sizeof(Zenyth::CameraData));
	commandBatch.TransitionBarrier(*m_sceneConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	commandBatch.CopyBufferRegion(*m_lightBuffer, m_frameIndex * m_lightBuffer->GetElementSize(), *m_lightUploadBuffer, 0, sizeof(Zenyth::LightData));
	commandBatch.TransitionBarrier(*m_lightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);


	commandBatch.SetRootParameter(0, m_meshConstantBuffer->GetCBV());
	commandBatch.SetRootParameter(1, m_cameraConstantBuffer->GetCBV());
	commandBatch.SetRootParameter(2, m_sceneConstantBuffer->GetCBV());
	commandBatch.SetRootParameter(6, m_lightBuffer->GetSRV());

	// will submit material, should be used
	m_meshRenderer->Submit(commandBatch);


	ImGui::Begin("Helper");
	ImGui::Text("FPS: %.2f", 1.0f / m_timer.GetElapsedSeconds());
	ImGui::Text("loaded files %i", Zenyth::Texture::loadedFile);

	// m_camera.OnImGui();

	ImGui::TreePush("Foo");
	ImGui::DragFloat3("Sun Direction", &m_directionalLight.direction.x, 0.01f);
	ImGui::ColorEdit3("Sun Color", &m_directionalLight.color.x);
	ImGui::TreePop();


	ImGui::End();
	m_imguiLayer->Render(commandList);

	commandBatch.TransitionBarrier(*color0, D3D12_RESOURCE_STATE_PRESENT, true);


	const auto fence = commandBatch.End();
	m_fenceValues[m_frameIndex] = fence;
}

void ModelGallery::MoveToNextFrame()
{
	auto& graphicsQueue = GetRenderer().GetCommandManager().GetGraphicsQueue();

	// Update the frame index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	graphicsQueue.WaitForFence(m_fenceValues[m_frameIndex]);
}

std::wstring ModelGallery::GetAssetFullPathW(const std::wstring& assetName)
{
	return WORKING_DIR L"resources/" + assetName;
}

std::string ModelGallery::GetAssetFullPath(const std::string& assetName)
{
	return WORKING_DIR "resources/" + assetName;
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
		GetRenderer().GetCommandManager().IdleGPU();

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
		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		optimizedClearValue.DepthStencil = { 0.0F, 0 };

		const auto depthTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		for (UINT n = 0; n < FrameCount; n++)
		{
			ComPtr<ID3D12Resource> backBuffer = nullptr;
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(backBuffer.GetAddressOf())), "Failed to get buffer");

			const auto& renderTarget = m_renderTargets[n];

			const auto color0 = renderTarget->GetTexture(Zenyth::AttachmentPoint::Color0);
			color0->Create(std::format(L"Render Target #{}", n), backBuffer, nullptr);

			const auto depthStencil = renderTarget->GetTexture(Zenyth::AttachmentPoint::DepthStencil);
			depthStencil->Create(L"Depth Stencil Texture", depthTextureDesc, &optimizedClearValue );
		}
	}
}
