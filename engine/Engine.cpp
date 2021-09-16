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
	}

	void Init(HINSTANCE hInstance)
	{
		Window::Create(hInstance);
		if(Window::Get()->IsRunning()) g_Engine = new GameEngine();
	}

	void Run()
	{
		if (g_Engine) g_Engine->Run();
	}

	void Deinit()
	{
		SAFE_DELETE(g_Engine);
		Window::Destroy();
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

	void ShowCursor(bool show)
	{
		Window::Get()->ShowCursor(show);
	}

	void Shutdown()
	{
		Window::Get()->Shutdown();
	}

	void ReloadShaders()
	{
		g_Engine->GetRenderer()->ReloadShaders();
	}
}