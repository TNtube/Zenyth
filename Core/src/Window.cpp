#include "pch.hpp"
#include "Window.hpp"

namespace Zenyth {

    uint32_t Window::s_windowCount = 0;

    Window::Window(const WindowDesc& desc)
        : m_width(desc.width)
        , m_height(desc.height)
        , m_resizable(desc.resizable)
        , m_hinst(::GetModuleHandleW(nullptr))
    {
        // Unique class name per window instance
        m_className = L"EngineWindow_" + std::to_wstring(s_windowCount++);

        RegisterWindowClass();
        CreateNativeWindow(desc);
    }

    Window::~Window() {
        if (m_hwnd) {
            ::DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
        ::UnregisterClassW(m_className.c_str(), m_hinst);
    }

    void Window::RegisterWindowClass() {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProcStatic;
        wc.hInstance = m_hinst;
        wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = m_className.c_str();
        wc.hIcon = ::LoadIconW(nullptr, IDI_APPLICATION);
        wc.hIconSm = ::LoadIconW(nullptr, IDI_APPLICATION);

        if (!::RegisterClassExW(&wc))
            throw std::runtime_error("Window::RegisterWindowClass failed");
    }

    void Window::CreateNativeWindow(const WindowDesc& desc) {
        DWORD style = WS_OVERLAPPEDWINDOW;
        if (!m_resizable)
            style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);

        RECT rc = { 0, 0,
                    static_cast<LONG>(m_width),
                    static_cast<LONG>(m_height) };
        ::AdjustWindowRect(&rc, style, FALSE);

        m_hwnd = ::CreateWindowExW(
            0,
            m_className.c_str(),
            desc.title.c_str(),
            style,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rc.right - rc.left,
            rc.bottom - rc.top,
            nullptr, nullptr,
            m_hinst,
            this   // passed to WM_NCCREATE → stored in GWLP_USERDATA
        );

        if (!m_hwnd)
            throw std::runtime_error("Window::CreateNativeWindow failed");

        ::ShowWindow(m_hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(m_hwnd);
    }

    bool Window::PumpMessages() {
        MSG msg{};
        // Drain the entire queue without blocking
        while (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                return false;
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
        return true;
    }

    LRESULT CALLBACK Window::WndProcStatic(HWND hwnd, UINT msg,
        WPARAM wparam, LPARAM lparam) {
        Window* self = nullptr;

        if (msg == WM_NCCREATE) {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lparam);
            self = static_cast<Window*>(cs->lpCreateParams);
            ::SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(self));
            self->m_hwnd = hwnd; // hwnd is valid from this point on
        }
        else {
            self = reinterpret_cast<Window*>(
                ::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        if (self)
            return self->WndProc(hwnd, msg, wparam, lparam);

        return ::DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    LRESULT Window::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        switch (msg) {
        // Window lifecycle
        case WM_CLOSE: {
            Event e;
            e.type = EventType::WindowClose;
            e.close = {};
            EmitEvent(e);
            // Don't call DestroyWindow here – let the Application decide
            return 0;
        }

        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;

        case WM_SIZE: {
            uint32_t w = LOWORD(lparam);
            uint32_t h = HIWORD(lparam);
            if (w != m_width || h != m_height) {
                m_width = w;
                m_height = h;
                Event e;
                e.type = EventType::WindowResize;
                e.resize = { w, h };
                EmitEvent(e);
            }
            return 0;
        }

        // Keyboard

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            Event e;
            e.type = EventType::KeyDown;
            e.key = { static_cast<uint32_t>(wparam),
                       (lparam & (1 << 30)) != 0 }; // bit 30 = previous key state
            EmitEvent(e);
            return 0;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP: {
            Event e;
            e.type = EventType::KeyUp;
            e.key = { static_cast<uint32_t>(wparam), false };
            EmitEvent(e);
            return 0;
        }

        // Mouse

        case WM_MOUSEMOVE: {
            Event e;
            e.type = EventType::MouseMove;
            e.mouseMove = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
            EmitEvent(e);
            return 0;
        }

        case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN: {
            uint8_t btn = (msg == WM_LBUTTONDOWN) ? 0
                : (msg == WM_RBUTTONDOWN) ? 1 : 2;
            ::SetCapture(hwnd); // track mouse outside window while button held
            Event e;
            e.type = EventType::MouseButtonDown;
            e.mouseButton = { btn, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
            EmitEvent(e);
            return 0;
        }

        case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP: {
            uint8_t btn = (msg == WM_LBUTTONUP) ? 0
                : (msg == WM_RBUTTONUP) ? 1 : 2;
            ::ReleaseCapture();
            Event e;
            e.type = EventType::MouseButtonUp;
            e.mouseButton = { btn, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
            EmitEvent(e);
            return 0;
        }

        case WM_MOUSEWHEEL: {
            Event e;
            e.type = EventType::MouseWheel;
            e.mouseWheel = { static_cast<float>(GET_WHEEL_DELTA_WPARAM(wparam))
                             / static_cast<float>(WHEEL_DELTA) };
            EmitEvent(e);
            return 0;
        }

        default:
            break;
        }

        return ::DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    void Window::EmitEvent(const Event& e) const {
        if (m_callback)
            m_callback(e);
    }

} // namespace Zenyth
