#include "core/Window.h"

#include <unordered_map>

#include "Config.h"

#include <debugapi.h>
#include <string>

#include "util/StringUtil.h"
#include "gui//GUI.h"

namespace GP
{
    Window* Window::s_Instance = nullptr;

    inline RECT GetClipRect()
    {
        RECT rc;
        GetWindowRect(Window::Get()->GetHandle(), &rc);
        long width = rc.right - rc.left;
        long height = rc.bottom - rc.top;
        rc.left += width / 4;
        rc.right -= width / 4;
        rc.top += height / 4;
        rc.bottom -= height / 4;
        return rc;
    }

    __declspec(dllexport) LRESULT CALLBACK ClipMouseEvent(int nCode, WPARAM wParam, LPARAM lParam)
    {
        static Window* wnd = Window::Get();
        RECT rc = GetClipRect();

        if (wnd->GetHandle() == GetActiveWindow())
            ClipCursor(&rc);

        return CallNextHookEx(0, nCode, wParam, lParam);
    }

    namespace WindowInput
    {
        static Vec2 s_MousePos = VEC2_ZERO;
        static Vec2 s_MouseDelta = VEC2_ZERO;

        static std::unordered_map<unsigned int, bool> s_InputDownMap;
        static std::unordered_map<unsigned int, bool> s_BeginFrameInputs;
        static std::unordered_map<unsigned int, bool> s_JustPressedInputMap;

        void InputFrameBegin()
        {
            static std::unordered_map<unsigned int, bool>::iterator it;
            for (it = s_InputDownMap.begin(); it != s_InputDownMap.end(); it++)
            {
                s_BeginFrameInputs[it->first] = it->second;
            }
        }

        void InputFrameEnd()
        {
            static std::unordered_map<unsigned int, bool>::iterator it;
            for (it = s_InputDownMap.begin(); it != s_InputDownMap.end(); it++)
            {
                const unsigned int key = it->first;
                const bool hasElement = s_BeginFrameInputs.find(key) == s_BeginFrameInputs.end();
                s_JustPressedInputMap[it->first] = (hasElement || s_BeginFrameInputs[key] != s_InputDownMap[key]) && s_InputDownMap[key];
            }
        }

        void OnMouseMoved(unsigned int mouseX, unsigned int mouseY)
        {
            static Window* wnd = Window::Get();
            Vec2 mousePosNormalized = Vec2((float)mouseX / wnd->GetWidth(), (float)mouseY / wnd->GetHeight());
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

        bool IsKeyJustPressed(unsigned int key)
        {
            if (s_JustPressedInputMap.find(key) == s_JustPressedInputMap.end()) return false;
            return s_JustPressedInputMap[key];
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

        if (g_GUI->HandleWndProc(hwnd, msg, wparam, lparam))
        {
            return true;
        }

        switch (msg)
        {
        case WM_SIZE:
        {
            if (Window::Get())
            {
                Window::Get()->SetWindowWidth(LOWORD(lparam));
                Window::Get()->SetWindowHeight(HIWORD(lparam));
            }
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
        case WM_ACTIVATE:
        {
            if (Window::Get()) Window::Get()->EnableMouseHook(wparam != WA_INACTIVE);
            break;
        }
        default:
            result = DefWindowProcW(hwnd, msg, wparam, lparam);
        }
        return result;
    }

    Window::Window(HINSTANCE instance, const std::string& title):
        m_Instance(instance)
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
            return;
        }

        RECT initialRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
        AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
        const LONG initialWidth = initialRect.right - initialRect.left;
        const LONG initialHeight = initialRect.bottom - initialRect.top;
        const std::wstring wTitle = StringUtil::ToWideString(title);

        m_Handle = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
            winClass.lpszClassName,
            wTitle.c_str(),
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            initialWidth,
            initialHeight,
            0, 0, m_Instance, 0);

        if (!m_Handle) {
            MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
            return;
        }

        m_Running = true;
    }

    Window::~Window()
    {
        EnableMouseHook(m_MouseHook);
    }

    void Window::Update(float dt)
    {
        WindowInput::s_MouseDelta = VEC2_ZERO;

        MSG message = {};
        while (PeekMessageW(&message, 0, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
                m_Running = false;
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    void Window::EnableMouseHook(bool enable)
    {
        // TODO: Enable this after fixing: "[Window] Retarded mouse after breaking into debugger" and "[Window] Retarded mouse after alt tab back to the game"
        return;

        if (enable)
        {
            if(!m_MouseHook) m_MouseHook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)ClipMouseEvent, m_Instance, 0);
        }
        else
        {
            if(m_MouseHook) UnhookWindowsHookEx(m_MouseHook);
            m_MouseHook = nullptr;
        }
    }

    void Window::ShowCursor(bool show)
    {
        ::ShowCursor(show);
        EnableMouseHook(show);
    }
}