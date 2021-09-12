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
		bool Init(HINSTANCE instance);
		void Update(float dt);

		void Shutdown() { m_Running = false; }

		inline bool IsRunning() const { return m_Running; }
		inline HWND GetHandle() const { return m_Handle; }

		unsigned int GetWidth();
		unsigned int GetHeight();

	private:
		bool m_Running = false;
		HWND m_Handle;
	};

	namespace WindowInput
	{
		bool IsKeyPressed(unsigned int key);
		Vec2 GetMousePos();
	}
}
