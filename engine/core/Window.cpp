#include "core/Window.h"

#include <unordered_map>

#include "Config.h"

#include <debugapi.h>
#include <string>

namespace GP
{
    // TODO: Make whole window static and add this to their members
    unsigned int s_WindowWidth = WINDOW_WIDTH;
    unsigned int s_WindowHeight = WINDOW_HEIGHT;

    unsigned int s_MouseClipped = false;
    static HHOOK s_MouseHook;
    static HWND s_WindowHandle;

    __declspec(dllexport) LRESULT CALLBACK MouseEvent(int nCode, WPARAM wParam, LPARAM lParam)
    {
        RECT rc;
        GetWindowRect(s_WindowHandle, &rc);

        if (s_MouseClipped)
            ClipCursor(&rc);

        return CallNextHookEx(s_MouseHook, nCode, wParam, lParam);
    }

    namespace WindowInput
    {
        static Vec2 s_MousePos = VEC2_ZERO;
        static Vec2 s_MouseDelta = VEC2_ZERO;

        static std::unordered_map<unsigned int, bool> s_InputDownMap;

        void OnMouseMoved(unsigned int mouseX, unsigned int mouseY)
        {
            Vec2 mousePosNormalized = Vec2((float)mouseX / s_WindowWidth, (float)mouseY / s_WindowHeight);
            s_MouseDelta = mousePosNormalized - s_MousePos;
            s_MousePos = mousePosNormalized;
        }

        void OnKeyStateChanged(unsigned int key, unsigned int state)
        {
            s_InputDownMap[key] = state == WM_KEYDOWN;
        }

        bool IsKeyPressed(unsigned int key)
        {
            if (s_InputDownMap.find(key) == s_InputDownMap.end()) return false;
            return s_InputDownMap[key];
        }

        Vec2 GetMousePos()
        {
            return s_MousePos;
        }

        Vec2 GetMouseDelta()
        {
            return s_MouseDelta;
        }
    }

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        LRESULT result = 0;
        switch (msg)
        {
        case WM_SIZE:
        {
            s_WindowWidth = LOWORD(lparam);
            s_WindowHeight = HIWORD(lparam);
            break;
        }
        case WM_MOUSEMOVE:
        {
            WindowInput::OnMouseMoved(LOWORD(lparam), HIWORD(lparam));
            break;
        }
        case WM_KEYUP:
        case WM_KEYDOWN:
        {
            WindowInput::OnKeyStateChanged(wparam, msg);
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        default:
            result = DefWindowProcW(hwnd, msg, wparam, lparam);
        }
        return result;
    }

    bool Window::Init(HINSTANCE instance)
    {
        WNDCLASSEXW winClass = {};
        winClass.cbSize = sizeof(WNDCLASSEXW);
        winClass.style = CS_HREDRAW | CS_VREDRAW;
        winClass.lpfnWndProc = &WndProc;
        winClass.hInstance = instance;
        winClass.hIcon = LoadIconW(0, IDI_APPLICATION);
        winClass.hCursor = LoadCursorW(0, IDC_ARROW);
        winClass.lpszClassName = L"MyWindowClass";
        winClass.hIconSm = LoadIconW(0, IDI_APPLICATION);

        if (!RegisterClassExW(&winClass)) {
            MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
            return false;
        }

        RECT initialRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
        AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
        LONG initialWidth = initialRect.right - initialRect.left;
        LONG initialHeight = initialRect.bottom - initialRect.top;

        m_Handle = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
            winClass.lpszClassName,
            L"DX11Graphics",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            initialWidth,
            initialHeight,
            0, 0, instance, 0);

        if (!m_Handle) {
            MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
            return false;
        }

        s_MouseHook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)MouseEvent, instance, 0);
        UnhookWindowsHookEx(s_MouseHook);
        s_WindowHandle = m_Handle;

        m_Running = true;
        return true;
    }

    void Window::Update(float dt)
    {
        MSG message = {};
        while (PeekMessageW(&message, 0, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
                m_Running = false;
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }


    unsigned int Window::GetWidth()
    {
        return s_WindowWidth;
    }

    unsigned int Window::GetHeight()
    {
        return s_WindowHeight;
    }

    void Window::ShowCursor(bool show)
    {
        ::ShowCursor(show);
        s_MouseClipped = !show;
    }
}