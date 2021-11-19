#include "Renderer.h"

#include <vector>
#include <string>

#include "core/Window.h"
#include "core/RenderPass.h"
#include "core/GlobalVariables.h"
#include "gui/GUI.h"
#include "gfx/GfxDevice.h"
#include "gfx/GfxBuffers.h"
#include "util/Timer.h"

namespace GP
{
    Renderer::Renderer()
    {
        g_Device = new GfxDevice();
        g_Device->Init();
        ASSERT(g_Device->IsInitialized(), "[Renderer] Device not initialized!");
        g_GUI->InitializeDefaultScene();

        m_GlobalsBuffer = new GfxConstantBuffer<CBEngineGlobals>();
    }

    Renderer::~Renderer()
    {
        for (RenderPass* renderPass : m_Schedule)
        {
            delete renderPass;
        }
        m_Schedule.clear();
        
        delete m_GlobalsBuffer;
        delete g_Device;
    }

    void Renderer::Reset()
    {
        for (RenderPass* renderPass : m_Schedule)
        {
            delete renderPass;
        }
        m_Schedule.clear();
        g_GUI->Reset();
    }

    void Renderer::Update(float dt)
    {
        // Update GUI
        g_GUI->Update(dt);

        // Update last render time
        static float timeUntilLastRender = 0.0f;
        timeUntilLastRender += dt;
        if (timeUntilLastRender > TICK)
        {
            m_ShouldRender = true;
            timeUntilLastRender = 0;
        }

        // Update globals
        static float timeSinceStart = 0;
        timeSinceStart += dt / 1000.0f;
        static CBEngineGlobals globals = {};
        globals.screenWidth = (float) Window::Get()->GetWidth();
        globals.screenHeight = (float) Window::Get()->GetHeight();
        globals.time = timeSinceStart;
        m_GlobalsBuffer->Upload(globals);
    }

    bool Renderer::RenderIfShould()
    {
        if (m_ShouldRender)
        {
            RenderFrame();
            m_ShouldRender = false;
            return true;
        }

        return false;
    }

    void Renderer::RenderFrame()
    {
        static Timer fpsTimer;
        fpsTimer.Start();

        GfxContext* context = g_Device->GetContext();
        context->Clear();
        for (RenderPass* renderPass : m_Schedule)
        {
            if (!renderPass->IsInitialized())
            {
                renderPass->Init(context);
                renderPass->SetInitialized(true);
            }
            renderPass->Render(context);
        }
        g_GUI->Render();
        g_Device->EndFrame();

        fpsTimer.Stop();
        GlobalVariables::CURRENT_FPS = (int) (1000.0f / fpsTimer.GetTimeMS());
    }

    void Renderer::ReloadShaders()
    {
        for (RenderPass* renderPass : m_Schedule)
        {
            renderPass->ReloadShaders();
        }
    }
}
