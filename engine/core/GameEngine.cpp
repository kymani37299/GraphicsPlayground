#include "GameEngine.h"

#include <chrono>

#include "Common.h"
#include "core/Window.h"
#include "core/Renderer.h"
#include "core/Controller.h"
#include "GfxCore.h"

namespace GP
{
	GameEngine::GameEngine(Window* window) :
		m_Window(window)
	{
		m_Renderer = new Renderer(window);
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
		ASSERT(m_Window->IsRunning(), "Trying to run an engine without a window!");

		GameLoop();
		m_FirstFrame = false;

		while (m_Window->IsRunning())
		{
			GameLoop();
		}
	}

	void GameEngine::GameLoop()
	{
		UpdateDT();
		m_Controller->UpdateInput(m_DT);
		m_Renderer->Update(m_DT);
		if (m_Renderer->RenderIfShould())
		{
			m_Window->Update(m_DT);
		}
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
	}


}