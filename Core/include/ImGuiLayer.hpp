#pragma once
#include "Renderer/DescriptorHeap.hpp"

class ImGuiLayer
{
public:
	ImGuiLayer(ID3D12Device* device, ID3D12CommandQueue* commandQueue);
	~ImGuiLayer();

	void Begin() const;
	void Render(ID3D12GraphicsCommandList* commandList) const;
	void End() const;

	void OnWindowSizeChanged(uint32_t width, uint32_t height) const;
};

