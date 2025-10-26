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

class ModelGallery final : public Zenyth::Application
{
public:
	ModelGallery(uint32_t width, uint32_t height, bool useWrapDevice);

	void OnInit() override;
	void Tick() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnDestroy() override;

	void OnWindowSizeChanged(int width, int height) override;

	void LoadSizeDependentResources() const;

private:
	float m_aspectRatio;


	struct SceneConstants
	{
		uint32_t activeLightCount;
		float time;
		float deltaTime;
		uint32_t shadowMapCount;

		DirectX::SimpleMath::Vector3 cameraPosition;
		float _pad1;

		DirectX::SimpleMath::Vector2 screenResolution;
		DirectX::SimpleMath::Vector2 _pad0;

	};

	DWORD m_callbackCookie{};

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;

	std::unique_ptr<Zenyth::Pipeline> m_pipelineGeometry;

	std::unique_ptr<Zenyth::RenderTarget> m_renderTargets[FrameCount];

	bool m_depthBoundsTestSupported = false;

	std::unique_ptr<Zenyth::ImGuiLayer> m_imguiLayer {};

	std::unique_ptr<Zenyth::UploadBuffer> m_lightUploadBuffer {};
	std::unique_ptr<Zenyth::StructuredBuffer> m_lightBuffer {};
	float m_lightSpeed = 5.0f;

	std::unique_ptr<Zenyth::UploadBuffer> m_cameraCpuBuffer {};
	std::unique_ptr<Zenyth::ConstantBuffer> m_cameraConstantBuffer {};

	std::unique_ptr<Zenyth::UploadBuffer> m_sceneUploadBuffer {};
	std::unique_ptr<Zenyth::ConstantBuffer> m_sceneConstantBuffer {};

	std::unique_ptr<Zenyth::UploadBuffer> m_meshUploadBuffer {};
	std::unique_ptr<Zenyth::ConstantBuffer> m_meshConstantBuffer;

	Zenyth::Transform m_meshTransform;
	std::unique_ptr<Zenyth::MeshRenderer> m_meshRenderer;

	// Synchronization objects.
	UINT m_frameIndex;
	uint64_t m_fenceValues[FrameCount] {};


	void LoadPipeline();
	void LoadAssets();

	void PopulateCommandList();

	void MoveToNextFrame();

	static std::wstring GetAssetFullPathW(const std::wstring& assetName);
	static std::string GetAssetFullPath(const std::string& assetName);

	Zenyth::Camera m_camera;
	// Rendering loop timer.
	DX::StepTimer							m_timer;

	// Input devices.
	std::unique_ptr<DirectX::Keyboard>		m_keyboard;
	std::unique_ptr<DirectX::Mouse>			m_mouse;


	Zenyth::LightData m_directionalLight;
};
