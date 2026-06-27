#pragma once

namespace Zenyth {
	enum class EventType {
		WindowClose,
		WindowResize,
		KeyDown,
		KeyUp,
		MouseMove,
		MouseButtonDown,
		MouseButtonUp,
		MouseWheel,
	};

	struct WindowCloseEvent {};
	struct WindowResizeEvent { uint32_t width, height; };
	struct KeyEvent { uint32_t keycode; bool repeat; };
	struct MouseMoveEvent { int32_t x, y; };
	struct MouseButtonEvent { uint8_t button; int32_t x, y; };  // button: 0=L,1=R,2=M
	struct MouseWheelEvent { float delta; };

	struct Event {
		EventType type;
		union {
			WindowCloseEvent  close;
			WindowResizeEvent resize;
			KeyEvent          key;
			MouseMoveEvent    mouseMove;
			MouseButtonEvent  mouseButton;
			MouseWheelEvent   mouseWheel;
		};
	};

	using EventCallback = std::function<void(const Event&)>;

	struct WindowDesc {
		std::wstring title = L"Engine";
		uint32_t     width = 1280;
		uint32_t     height = 720;
		bool         resizable = true;
	};
	
	class Window {
	public:
		explicit Window(const WindowDesc& desc);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		[[nodiscard]] bool PumpMessages();

		void SetEventCallback(EventCallback cb) { m_callback = std::move(cb); }

		[[nodiscard]] HWND     GetHandle() const { return m_hwnd; }
		[[nodiscard]] uint32_t GetWidth()  const { return m_width; }
		[[nodiscard]] uint32_t GetHeight() const { return m_height; }

	private:
		void RegisterWindowClass();
		void CreateNativeWindow(const WindowDesc& desc);

		static LRESULT CALLBACK WndProcStatic(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		void EmitEvent(const Event& e) const;

		HWND          m_hwnd = nullptr;
		HINSTANCE     m_hInst = nullptr;
		uint32_t      m_width = 0;
		uint32_t      m_height = 0;
		bool          m_resizable = true;
		EventCallback m_callback;

		std::wstring  m_className;

		static uint32_t s_windowCount;
	};

} // namespace Zenyth
