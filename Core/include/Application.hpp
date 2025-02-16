#pragma once
#include "Transform.hpp"

namespace Zenyth {

	class Application
	{
	public:
		Application(uint32_t width, uint32_t height, bool useWrapDevice);
		virtual ~Application() = default;

		[[nodiscard]] uint32_t GetWidth() const { return m_width; }
		[[nodiscard]] uint32_t GetHeight() const { return m_height; }

		virtual void OnInit() = 0;
		virtual void Tick() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
		virtual void OnDestroy() = 0;

		virtual void OnWindowSizeChanged(int width, int height) = 0;
	protected:
		static void GetHardwareAdapter(
			IDXGIFactory1* pFactory,
			IDXGIAdapter1** ppAdapter,
			bool requestHighPerformanceAdapter = false);

		static constexpr UINT FrameCount = 3;

		uint32_t m_width;
		uint32_t m_height;

		bool m_useWarpDevice = false;
	};
}