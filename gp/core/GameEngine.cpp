#include "GameEngine.h"

#include <chrono>

#include "Common.h"
#include "core/Window.h"
#include "core/Renderer.h"
#include "core/Controller.h"
#include "core/Loading.h"
#include "gfx/GfxDevice.h"

namespace GP
{
	LoadingThread* g_LoadingThread;
	PoisonPillTask* PoisonPillTask::s_Instance = nullptr;

	GameEngine::GameEngine()
	{
		m_Renderer = new Renderer();
		m_Controller = new Controller();
		g_LoadingThread = new LoadingThread();
	}

	GameEngine::~GameEngine()
	{
		delete g_LoadingThread;
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

		GameLoop();
		m_FirstFrame = false;

		while (wnd->IsRunning())
		{
			WindowInput::InputFrameBegin();
			GameLoop();
			WindowInput::InputFrameEnd();
		}
	}

	void GameEngine::Reset()
	{
		if (g_LoadingThread) g_LoadingThread->ResetAndWait();
		m_Renderer->Reset();
	}

	void GameEngine::GameLoop()
	{
		UpdateDT();
		Window::Get()->Update(m_DT);
		m_Controller->UpdateInput(m_DT);
		m_Renderer->Update(m_DT);
		m_Renderer->RenderIfShould();
		Logger::Get()->DispatchLogs();
	}

	namespace Input
	{
		bool IsKeyPressed(unsigned int key)
		{
			return WindowInput::IsKeyPressed(key);
		}

		bool IsKeyJustPressed(unsigned int key)
		{
			return WindowInput::IsKeyJustPressed(key);
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