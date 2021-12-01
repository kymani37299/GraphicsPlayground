#include "ScopedOperations.h"

#include  "gfx/GfxDevice.h"

namespace GP
{
    BeginRenderPassScoped::BeginRenderPassScoped(const std::string& debugName)
    {
        // Using immediate context here because this function can be called only from main thread
        g_Device->GetImmediateContext()->BeginPass(debugName);
    }

    BeginRenderPassScoped::~BeginRenderPassScoped()
    {
        g_Device->GetImmediateContext()->EndPass();
    }

    RenderTargetScoped::RenderTargetScoped(GfxContext* context, GfxRenderTarget* rt, GfxRenderTarget* ds) :
        m_Context(context),
        m_LastRT(context->GetRenderTarget()),
        m_LastDS(context->GetDepthStencil())
    {
        // First we must detach DS, in case that new render target size doesn't match old DS size
        m_Context->SetDepthStencil(nullptr);
        m_Context->SetRenderTarget(rt);
        m_Context->SetDepthStencil(ds);
    }

    RenderTargetScoped::~RenderTargetScoped()
    {
        m_Context->SetDepthStencil(nullptr);

        m_Context->SetRenderTarget(m_LastRT);
        m_Context->SetDepthStencil(m_LastDS);
    }
}