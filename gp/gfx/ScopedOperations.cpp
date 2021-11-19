#include "ScopedOperations.h"

#include  "gfx/GfxDevice.h"

namespace GP
{
    BeginRenderPassScoped::BeginRenderPassScoped(const std::string& debugName)
    {
        g_Device->GetContext()->BeginPass(debugName);
    }

    BeginRenderPassScoped::~BeginRenderPassScoped()
    {
        g_Device->GetContext()->EndPass();
    }

    DeviceStateScoped::DeviceStateScoped(GfxDeviceState* state) :
        m_LastState(g_Device->GetContext()->GetState())
    {
        g_Device->GetContext()->BindState(state);
    }

    DeviceStateScoped::~DeviceStateScoped()
    {
        g_Device->GetContext()->BindState(m_LastState);
    }

    RenderTargetScoped::RenderTargetScoped(GfxRenderTarget* rt, GfxRenderTarget* ds) :
        m_LastRT(g_Device->GetContext()->GetRenderTarget()),
        m_LastDS(g_Device->GetContext()->GetDepthStencil())
    {
        // First we must detach DS, in case that new render target size doesn't match old DS size
        g_Device->GetContext()->SetDepthStencil(nullptr);

        g_Device->GetContext()->SetRenderTarget(rt);
        g_Device->GetContext()->SetDepthStencil(ds);
    }

    RenderTargetScoped::~RenderTargetScoped()
    {
        g_Device->GetContext()->SetDepthStencil(nullptr);

        g_Device->GetContext()->SetRenderTarget(m_LastRT);
        g_Device->GetContext()->SetDepthStencil(m_LastDS);
    }
}