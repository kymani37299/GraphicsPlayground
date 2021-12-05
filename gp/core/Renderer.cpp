#include "Renderer.h"

#include <vector>
#include <string>

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
        for (RenderPass* renderPass : m_RenderPasses)
        {
            delete renderPass;
        }
        m_RenderPasses.clear();
        m_Schedule.clear();
        
        delete m_GlobalsBuffer;
        delete g_Device;
    }

    void Renderer::Reset()
    {
        for (RenderPass* renderPass : m_RenderPasses)
        {
            delete renderPass;
        }
        m_RenderPasses.clear();
        m_Schedule.clear();
        g_GUI->Reset();
    }

    void Renderer::Update(float dt)
    {
        static GPConfig& gpConfig = GlobalVariables::GP_CONFIG;

        // Update GUI
        g_GUI->Update(dt);

        // Check for window resized
        if (gpConfig.WindowSizeDirty)
        {
            g_Device->RecreateSwapchain();
            // TODO: GUI needs some recreation
            for (RenderPass* renderPass : m_RenderPasses)
                renderPass->OnWindowResized(g_Device->GetImmediateContext(), gpConfig.WindowWidth, gpConfig.WindowHeight);

            gpConfig.WindowSizeDirty = false;
        }

        // Update last render time
        static float timeUntilLastRender = 0.0f;
        timeUntilLastRender += dt / 1000.0f;
        if (timeUntilLastRender > 1.0f / GlobalVariables::GP_CONFIG.FPS)
        {
            m_ShouldRender = true;
            timeUntilLastRender = 0;
        }

        // Update globals
        static float timeSinceStart = 0;
        timeSinceStart += dt / 1000.0f;
        static CBEngineGlobals globals = {};
        globals.screenWidth = (float)   gpConfig.WindowWidth;
        globals.screenHeight = (float) gpConfig.WindowHeight;
        globals.time = timeSinceStart;

        g_Device->GetImmediateContext()->UploadToBuffer(m_GlobalsBuffer, globals);
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

        GfxContext* context = g_Device->GetImmediateContext();
        context->Clear();

        for (RenderPass* renderPass : m_RenderPasses)
        {
            if (!renderPass->IsInitialized())
            {
                renderPass->Init(context);
                renderPass->SetInitialized(true);
            }
        }

        for (RenderPass* renderPass : m_Schedule) renderPass->Render(context);

        g_GUI->Render();
        g_Device->EndFrame();

        fpsTimer.Stop();
        GlobalVariables::CURRENT_FPS = (int) (1000.0f / fpsTimer.GetTimeMS());
    }

    void Renderer::ReloadShaders()
    {
        for (RenderPass* renderPass : m_RenderPasses)
        {
            renderPass->ReloadShaders();
        }
    }
}
