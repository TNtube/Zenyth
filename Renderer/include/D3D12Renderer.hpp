#pragma once
#include "IRenderer.hpp"


namespace Zenyth {
	class D3D12Renderer : public IRenderer {
		void Init(HWND hwnd, uint32_t width, uint32_t height) override;
		void BeginFrame() override;
		void EndFrame() override;
		void Resize(uint32_t width, uint32_t height) override;
	};
}
