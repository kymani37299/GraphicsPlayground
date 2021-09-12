#include "Renderer.h"

#include <vector>
#include <string>

#include "Config.h"
#include "core/Core.h"
#include "core/Window.h"

#include "RenderPass.h"
#include "GfxCore.h"

namespace GP
{
    extern GfxDevice* g_Device;

    Renderer::Renderer(Window* window)
    {
        g_Device = new GfxDevice(window);
        ASSERT(g_Device->IsInitialized(), "[Renderer] Device not initialized!");
    }

    Renderer::~Renderer()
    {
        for (RenderPass* renderPass : m_Schedule)
        {
            delete renderPass;
        }
        m_Schedule.clear();
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
