#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>

#include "Common.h"

namespace GP
{
	class Window
	{
	public:
		inline static void Create(HINSTANCE instance) { s_Instance = new Window(instance); }
		inline static Window* Get() { return s_Instance; }
		inline static void Destroy() { SAFE_DELETE(s_Instance); }

	private:
		static Window* s_Instance;

	public:
		~Window();

		void Update(float dt);

		void Shutdown() { m_Running = false; }

		inline bool IsRunning() const { return m_Running; }
		unsigned int GetWidth() const { return m_WindowWidth; }
		unsigned int GetHeight() const { return m_WindowHeight; }

		inline HWND GetHandle() const { return m_Handle; }

		void SetWindowWidth(unsigned int width) { m_WindowWidth = width; }
		void SetWindowHeight(unsigned int height) { m_WindowHeight = height; }

		void EnableMouseHook(bool enable);
		void ShowCursor(bool show);

	private:
		Window(HINSTANCE instance);

	private:
		bool m_Running = false;

		unsigned int m_WindowWidth = WINDOW_WIDTH;
		unsigned int m_WindowHeight = WINDOW_HEIGHT;

		HINSTANCE m_Instance;
		HWND m_Handle;
		HHOOK m_MouseHook = nullptr;
	};

	namespace WindowInput
	{
		bool IsKeyPressed(unsigned int key);
		Vec2 GetMousePos();
		Vec2 GetMouseDelta();
	}
}
