#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>

#include "Common.h"
#include "core/GlobalVariables.h"

namespace GP
{
	class Window
	{
	public:
		inline static void Create(HINSTANCE instance, const std::string& title) { s_Instance = new Window(instance, title); }
		inline static Window* Get() { return s_Instance; }
		inline static void Destroy() { SAFE_DELETE(s_Instance); }

	private:
		static Window* s_Instance;

	public:
		void Update(float dt);

		void Shutdown() { m_Running = false; }

		inline bool IsRunning() const { return m_Running; }
		inline HWND GetHandle() const { return m_Handle; }
		inline bool IsCursorShown() const { return  m_ShowCursor; }

		void ShowCursor(bool show);

	private:
		Window(HINSTANCE instance, const std::string& title);

	private:
		bool m_Running = false;
		bool m_ShowCursor = true;

		HINSTANCE m_Instance;
		HWND m_Handle;
	};

	namespace WindowInput
	{
		void InputFrameBegin();
		void InputFrameEnd();

		bool IsKeyPressed(unsigned int key);
		bool IsKeyJustPressed(unsigned int key);
		Vec2 GetMousePos();
		Vec2 GetMouseDelta();
	}
}
