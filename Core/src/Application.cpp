#include "pch.hpp"
#include "Application.hpp"

namespace Zenyth {
	Application::Application(const AppDesc& desc)
		: m_desc(desc)
	{
		WindowDesc wd;
		wd.title = desc.title;
		wd.width = desc.width;
		wd.height = desc.height;
		wd.resizable = desc.resizable;

		m_window = std::make_unique<Window>(wd);
		m_timer = std::make_unique<Timer>();

		m_window->SetEventCallback([this](const Event& e) {
			OnWindowEvent(e);
		});
	}

	void Application::SetRenderer(std::unique_ptr<IRenderer> renderer) {
		m_renderer = std::move(renderer);
	}

	void Application::Run() {
		if (!m_window)
			throw std::runtime_error("Application::Run : window not created");
		if (!m_renderer)
			throw std::runtime_error("Application::Run : renderer not created");

		m_renderer->Init(m_window->GetHandle(),
			m_window->GetWidth(),
			m_window->GetHeight());

		m_timer->Reset();
		m_running = true;

		OnInit();

		while (m_running) {
			if (!m_window->PumpMessages())
				m_running = false;
			if (!m_running) break;

			const float dt = m_timer->Tick();
			OnUpdate(dt);

			m_renderer->BeginFrame();
			OnRender();
			m_renderer->EndFrame();
		}

		OnShutdown();
		
		m_renderer.reset();
	}

	void Application::OnWindowEvent(const Event& e) {
		OnEvent(e);
	}

	void Application::OnEvent(const Event& e) {
		switch (e.type) {

		case EventType::WindowClose:
			Stop();
			break;

		case EventType::WindowResize:
			if (e.resize.width > 0 && e.resize.height > 0 && m_renderer)
				m_renderer->Resize(e.resize.width, e.resize.height);
			break;

		default:
			break;
		}
	}
} // namespace Zenyth
