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
		inline HWND GetHandle() const { return m_Handle; }
		inline bool IsMouseClipped() const { return m_MouseClipped; }
		unsigned int GetWidth() const { return m_WindowWidth; }
		unsigned int GetHeight() const { return m_WindowHeight; }

		void SetWindowWidth(unsigned int width) { m_WindowWidth = width; }
		void SetWindowHeight(unsigned int height) { m_WindowHeight = height; }

		void ShowCursor(bool show);

	private:
		Window(HINSTANCE instance);

	private:
		bool m_Running = false;

		unsigned int m_WindowWidth = WINDOW_WIDTH;
		unsigned int m_WindowHeight = WINDOW_HEIGHT;
		unsigned int m_MouseClipped = false;

		HWND m_Handle;
		HHOOK m_MouseHook;
	};

	namespace WindowInput
	{
		bool IsKeyPressed(unsigned int key);
		Vec2 GetMousePos();
		Vec2 GetMouseDelta();
	}
}
