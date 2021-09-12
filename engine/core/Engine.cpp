#include "core/Engine.h"

#include <chrono>

#include "core/Core.h"
#include "core/Window.h"
#include "gfx/Renderer.h"

//#include "DemoScene.h"

Engine::Engine(Window* window):
	m_Window(window)
{
	m_Renderer = new Renderer(window);
}

Engine::~Engine()
{
	delete m_Renderer;
}

void Engine::UpdateDT()
{
	static auto t_before = std::chrono::high_resolution_clock::now();
	auto t_now = std::chrono::high_resolution_clock::now();
	m_DT = std::chrono::duration<float, std::milli>(t_now - t_before).count();
	t_before = t_now;
}

void Engine::Run()
{
	ASSERT(m_Window->IsRunning(), "Trying to run an engine without a window!");

	GameLoop();
	m_FirstFrame = false;

	while (m_Window->IsRunning())
	{
		GameLoop();
	}
}

void Engine::GameLoop()
{
	UpdateDT();
	UpdateInput();
	m_Renderer->Update(m_DT);
	if (m_Renderer->RenderIfShould())
	{
		m_Window->Update(m_DT);
	}
}

void Engine::UpdateInput()
{
	static const Vec3 UP_DIR = Vec3(0.0f, 1.0f, 0.0f);
	static const Vec3 RIGHT_DIR = Vec3(1.0f, 0.0f, 0.0f);
	static const Vec3 FORWARD_DIR = Vec3(0.0f, 0.0f, -1.0f);

	if (WindowInput::IsKeyPressed(VK_ESCAPE))
	{
		m_Window->Shutdown();
		return;
	}

	Vec3 moveDir = VEC3_ZERO;
	Vec3 rotation = VEC3_ZERO;

	if (WindowInput::IsKeyPressed('W'))
	{
		moveDir += FORWARD_DIR;
	}
	
	if (WindowInput::IsKeyPressed('S'))
	{
		moveDir -= FORWARD_DIR;
	}
	
	if (WindowInput::IsKeyPressed('A'))
	{
		moveDir -= RIGHT_DIR;
	}
	
	if (WindowInput::IsKeyPressed('D'))
	{
		moveDir += RIGHT_DIR;
	}
	
	if (WindowInput::IsKeyPressed('Q'))
	{
		moveDir += UP_DIR;
	}
	
	if (WindowInput::IsKeyPressed('E'))
	{
		moveDir -= UP_DIR;
	}

	if (WindowInput::IsKeyPressed(VK_UP))
	{
		rotation.x += 1.0f;
	}

	if (WindowInput::IsKeyPressed(VK_DOWN))
	{
		rotation.x -= 1.0f;
	}

	if (WindowInput::IsKeyPressed(VK_LEFT))
	{
		rotation.y -= 1.0f;
	}

	if (WindowInput::IsKeyPressed(VK_RIGHT))
	{
		rotation.y += 1.0f;
	}

#ifdef DEBUG
	if (WindowInput::IsKeyPressed('R'))
	{
		m_Renderer->ReloadShaders();
	}
#endif
}