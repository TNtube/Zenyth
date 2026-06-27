#pragma once

namespace Zenyth {

	class IRenderer {
	public:
		virtual ~IRenderer() = default;
		virtual void Init(HWND hwnd, uint32_t width, uint32_t height) = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
	};

}