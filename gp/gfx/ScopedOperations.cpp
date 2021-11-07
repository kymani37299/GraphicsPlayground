#include "ScopedOperations.h"

#include  "gfx/GfxDevice.h"

namespace GP
{
    BeginRenderPassScoped::BeginRenderPassScoped(const std::string& debugName)
    {
        g_Device->BeginPass(debugName);
    }

    BeginRenderPassScoped::~BeginRenderPassScoped()
    {
        g_Device->EndPass();
    }

    DeviceStateScoped::DeviceStateScoped(GfxDeviceState* state) :
        m_LastState(g_Device->GetState())
    {
        g_Device->BindState(state);
    }

    DeviceStateScoped::~DeviceStateScoped()
    {
        g_Device->BindState(m_LastState);
    }

    RenderTargetScoped::RenderTargetScoped(GfxRenderTarget* rt, GfxRenderTarget* ds) :
        m_LastRT(g_Device->GetRenderTarget()),
        m_LastDS(g_Device->GetDepthStencil())
    {
        // First we must detach DS, in case that new render target size doesn't match old DS size
        g_Device->SetDepthStencil(nullptr);

        g_Device->SetRenderTarget(rt);
        g_Device->SetDepthStencil(ds);
    }

    RenderTargetScoped::~RenderTargetScoped()
    {
        g_Device->SetDepthStencil(nullptr);

        g_Device->SetRenderTarget(m_LastRT);
        g_Device->SetDepthStencil(m_LastDS);
    }
}