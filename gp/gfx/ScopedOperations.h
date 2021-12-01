#pragma once

#include "gfx/GfxCommon.h"

#include <string>

#define GP_SCOPED_PROFILE(DebugName) ::GP::BeginRenderPassScoped JOIN(_rps, __LINE__)(DebugName)
#define GP_SCOPED_RT(Context, RenderTarget, DepthStencil)  ::GP::RenderTargetScoped JOIN(_rts, __LINE__)(Context, RenderTarget, DepthStencil)

namespace GP
{
	class GfxContext;
	class GfxRenderTarget;

	class BeginRenderPassScoped
	{
	public:
		GP_DLL BeginRenderPassScoped(const std::string& debugName);
		GP_DLL ~BeginRenderPassScoped();
	};

	class RenderTargetScoped
	{
	public:
		GP_DLL RenderTargetScoped(GfxContext* context, GfxRenderTarget* rt, GfxRenderTarget* ds = nullptr);
		GP_DLL ~RenderTargetScoped();

	private:
		GfxContext* m_Context;
		GfxRenderTarget* m_LastRT;
		GfxRenderTarget* m_LastDS;
	};
}