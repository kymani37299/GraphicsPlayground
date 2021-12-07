#include "GP.h"

#include "core/GameEngine.h"
#include "core/Window.h"
#include "core/Renderer.h"
#include "core/GlobalVariables.h"
#include "gui/GUI.h"

#include "defaults/DefaultController.h"

namespace GP
{
	namespace
	{
		GameEngine* g_Engine = nullptr;
	}

	void Init(HINSTANCE hInstance, unsigned int windowWidth, unsigned int windowHeight, const std::string& windowTitle, unsigned int fps, bool vsync)
	{
		GPConfig& gpConfig = GlobalVariables::GP_CONFIG;
		gpConfig.WindowWidth = windowWidth;
		gpConfig.WindowHeight = windowHeight;
		gpConfig.FPS = fps;
		gpConfig.VSYNC = vsync;

		Window::Create(hInstance, windowTitle);
		if(Window::Get()->IsRunning()) g_Engine = new GameEngine();
	}

	void Run()
	{
		if (g_Engine) g_Engine->Run();
	}

	void Reset()
	{
		if (g_Engine) g_Engine->Reset();
	}

	void Deinit()
	{
		SAFE_DELETE(g_Engine);
		Window::Destroy();
	}

	void SetController(Controller* controller)
	{
		g_Engine->SetController(controller);
	}

	void AddRenderPass(RenderPass* renderPass)
	{
		g_Engine->GetRenderer()->AddRenderPass(renderPass);
	}

	void ShowGui(bool show)
	{
		g_GUI->SetVisible(show);
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

	GfxConstantBuffer<CBEngineGlobals>* GetGlobalsBuffer()
	{
		return g_Engine->GetRenderer()->GetGlobalsBuffer();
	}
}