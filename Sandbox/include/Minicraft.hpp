#pragma once

#include "Application.hpp"
#include "Renderer/Buffers.hpp"
#include "Renderer/Texture.hpp"
#include "Camera.hpp"
#include "World.hpp"
#include "Renderer/DescriptorHeap.hpp"
#include "ImGuiLayer.hpp"
#include "StepTimer.hpp"
#include "Renderer/DepthStencilBuffer.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/PixelBuffer.hpp"

class Minicraft final : public Zenyth::Application
{
public:
	Minicraft(uint32_t width, uint32_t height, bool useWrapDevice);

	void OnInit() override;
	void Tick() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnDestroy() override;

	void OnWindowSizeChanged(int width, int height) override;

private:
	float m_aspectRatio;


	struct SceneConstantBuffer
	{
		DirectX::SimpleMath::Matrix model;
	};

	DWORD m_callbackCookie{};

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	std::unique_ptr<Zenyth::DescriptorHeap> m_rtvHeap {};
	std::unique_ptr<Zenyth::DescriptorHeap> m_dsvHeap {};
	std::unique_ptr<Zenyth::DescriptorHeap> m_resourceHeap {};
	std::unique_ptr<Zenyth::DepthStencilBuffer> m_depthStencilBuffer;

	std::unique_ptr<Zenyth::PixelBuffer> m_colorBuffer;
	std::unique_ptr<Zenyth::PixelBuffer> m_normalBuffer;
	std::unique_ptr<Zenyth::PixelBuffer> m_renderTargets[FrameCount];

	std::unique_ptr<Zenyth::Pipeline> m_pipelineGeometry;
	std::unique_ptr<Zenyth::Pipeline> m_pipelinePresent;

	std::unique_ptr<Zenyth::VertexBuffer> m_presentVertexBuffer;
	std::unique_ptr<Zenyth::IndexBuffer> m_presentIndexBuffer;

	bool m_depthBoundsTestSupported = false;


	// App resources.
	std::unique_ptr<Zenyth::ImGuiLayer> m_imguiLayer {};

	std::unique_ptr<Zenyth::Texture> m_tileset {};
	std::unique_ptr<Zenyth::Texture> m_tilesetNormal {};
	std::unique_ptr<Zenyth::UploadBuffer> m_cameraCpuBuffer {};
	std::unique_ptr<Zenyth::ConstantBuffer> m_cameraConstantBuffer {};

	std::unique_ptr<World> m_world {};

	// Synchronization objects.
	UINT m_frameIndex;
	uint64_t m_fenceValues[FrameCount] {};


	void LoadPipeline();
	void LoadAssets();

	void PopulateCommandList();

	void MoveToNextFrame();

	void LoadSizeDependentResources() const;

	static std::wstring GetAssetFullPath(const std::wstring& assetName);

	Zenyth::Camera m_camera;
	// Rendering loop timer.
	DX::StepTimer							m_timer;

	// Input devices.
	std::unique_ptr<DirectX::Keyboard>		m_keyboard;
	std::unique_ptr<DirectX::Mouse>			m_mouse;
};
