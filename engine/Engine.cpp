#include "Engine.h"

#include "core/GameEngine.h"
#include "core/Window.h"
#include "core/Renderer.h"

#include "defaults/DefaultController.h"

namespace GP
{
	namespace
	{
		GameEngine* g_Engine = nullptr;
		Window* g_Window = nullptr;
	}

	void Init(HINSTANCE hInstance)
	{
		g_Window = new Window();
		bool success = g_Window->Init(hInstance);
		if (success) g_Engine = new GameEngine(g_Window);
	}

	void Run()
	{
		if (g_Engine) g_Engine->Run();
	}

	void Deinit()
	{
		SAFE_DELETE(g_Engine);
		SAFE_DELETE(g_Window);
	}

	void SetDefaultController(Camera* camera)
	{
		g_Engine->SetController(new DefaultController(*camera));
	}

	void SetController(Controller* controller)
	{
		g_Engine->SetController(controller);
	}

	void AddRenderPass(RenderPass* renderPass)
	{
		g_Engine->GetRenderer()->AddRenderPass(renderPass);
	}

	void Shutdown()
	{
		g_Window->Shutdown();
	}

	void ReloadShaders()
	{
		g_Engine->GetRenderer()->ReloadShaders();
	}
}