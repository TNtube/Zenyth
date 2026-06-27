#pragma once

#include "Window.hpp"
#include "Timer.hpp"
#include "IRenderer.hpp"

namespace Zenyth {

	struct AppDesc {
		std::wstring title = L"Engine";
		uint32_t     width = 1280;
		uint32_t     height = 720;
		bool         resizable = true;
	};

	class Application {
	public:
		explicit Application(const AppDesc& desc);
		virtual ~Application() = default;

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;

		// takes ownership
		void SetRenderer(std::unique_ptr<IRenderer> renderer);
		void Run();
		void Stop() { m_running = false; }

		[[nodiscard]] Window& GetWindow() const { return *m_window; }
		[[nodiscard]] Timer& GetTimer() const { return *m_timer; }
		[[nodiscard]] IRenderer* GetRenderer() const { return m_renderer.get(); }

	protected:
		virtual void OnInit() {}
		virtual void OnShutdown() {}
		virtual void OnUpdate(float dt) = 0;
		virtual void OnRender() = 0;
		virtual void OnEvent(const Event& e);

	private:
		void OnWindowEvent(const Event& e);

		std::unique_ptr<Window>    m_window;
		std::unique_ptr<Timer>     m_timer;
		std::unique_ptr<IRenderer> m_renderer;

		bool     m_running = false;
		AppDesc  m_desc;
	};

} // namespace Zenyth
