#pragma once

#include "Application.hpp"
#include "Renderer/Buffers.hpp"
#include "Camera.hpp"
#include "ImGuiLayer.hpp"
#include "StepTimer.hpp"
#include "Data/Light.hpp"
#include "Data/Transform.hpp"
#include "Renderer/MeshRenderer.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/SwapChain.hpp"

class ModelGallery final : public Application
{
public:
	ModelGallery(uint32_t width, uint32_t height, bool useWrapDevice);

	void OnInit() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnDestroy() override;

	void OnWindowSizeChanged(int width, int height) override;

	void LoadSizeDependentResources();

private:
	float m_aspectRatio;

	DWORD m_callbackCookie{};

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	std::unique_ptr<SwapChain> m_swapChain;

	std::unique_ptr<Pipeline> m_pipelineGeometry;

	Texture m_depthStencilTexture;

	bool m_depthBoundsTestSupported = false;

	std::unique_ptr<ImGuiLayer> m_imguiLayer {};

	std::unique_ptr<StructuredBuffer> m_lightBuffer {};
	float m_lightSpeed = 5.0f;

	std::unique_ptr<ConstantBuffer> m_cameraConstantBuffer {};
	std::unique_ptr<ConstantBuffer> m_sceneConstantBuffer {};
	std::unique_ptr<ConstantBuffer> m_meshConstantBuffer;

	Transform m_meshTransform;
	std::unique_ptr<MeshRenderer> m_meshRenderer;
	std::unique_ptr<MeshRenderer> m_groundRenderer;


	void LoadPipeline();
	void LoadAssets();

	static std::wstring GetAssetFullPathW(const std::wstring& assetName);
	static std::string GetAssetFullPath(const std::string& assetName);

	Camera m_camera;
	// Rendering loop timer.
	StepTimer						m_timer;

	// Input devices.
	std::unique_ptr<DirectX::Keyboard>		m_keyboard;
	std::unique_ptr<DirectX::Mouse>			m_mouse;


	LightData m_directionalLight;
};
