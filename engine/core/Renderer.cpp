#include "Renderer.h"

#include <vector>
#include <string>

#include "core/Window.h"

#include "RenderPass.h"
#include "GfxCore.h"

namespace GP
{
    extern GfxDevice* g_Device;

    Renderer::Renderer()
    {
        g_Device = new GfxDevice();
        g_Device->Init();
        ASSERT(g_Device->IsInitialized(), "[Renderer] Device not initialized!");

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

    void Renderer::InitRenderPasses()
    {
        for (RenderPass* renderPass : m_Schedule)
        {
            renderPass->Init(g_Device);
        }
    }

    void Renderer::Update(float dt)
    {
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
        g_Device->Clear();
        for (RenderPass* renderPass : m_Schedule)
        {
            renderPass->Render(g_Device);
        }
        g_Device->Present();
    }
    void Renderer::ReloadShaders()
    {
        for (RenderPass* renderPass : m_Schedule)
        {
            renderPass->ReloadShaders();
        }
    }
}
