#include "GameEngine.h"

#include <chrono>

#include "Common.h"
#include "core/Window.h"
#include "core/Renderer.h"
#include "core/Controller.h"
#include "gfx/GfxCore.h"

namespace GP
{
	GameEngine::GameEngine()
	{
		m_Renderer = new Renderer();
		m_Controller = new Controller();
	}

	GameEngine::~GameEngine()
	{
		delete m_Renderer;
		delete m_Controller;
	}

	void GameEngine::UpdateDT()
	{
		static auto t_before = std::chrono::high_resolution_clock::now();
		auto t_now = std::chrono::high_resolution_clock::now();
		m_DT = std::chrono::duration<float, std::milli>(t_now - t_before).count();
		t_before = t_now;
	}

	void GameEngine::Run()
	{
		Window* wnd = Window::Get();
		ASSERT(wnd->IsRunning(), "Trying to run an engine without a window!");

		m_Renderer->InitRenderPasses();
		GameLoop();
		m_FirstFrame = false;

		while (wnd->IsRunning())
		{
			GameLoop();
		}
	}

	void GameEngine::GameLoop()
	{
		UpdateDT();
		Window::Get()->Update(m_DT);
		m_Controller->UpdateInput(m_DT);
		m_Renderer->Update(m_DT);
		m_Renderer->RenderIfShould();
	}

	namespace Input
	{
		bool IsKeyPressed(unsigned int key)
		{
			return WindowInput::IsKeyPressed(key);
		}

		Vec2 GetMousePos()
		{
			return WindowInput::GetMousePos();
		}

		Vec2 GetMouseDelta()
		{
			return WindowInput::GetMouseDelta();
		}
	}


}