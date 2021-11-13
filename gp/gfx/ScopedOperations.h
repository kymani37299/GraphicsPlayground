#pragma once

#include "gfx/GfxCommon.h"

#include <string>

#define GP_SCOPED_PROFILE(DebugName) ::GP::BeginRenderPassScoped JOIN(_rps, __LINE__)(DebugName)
#define GP_SCOPED_RT(RenderTarget, DepthStencil)  ::GP::RenderTargetScoped JOIN(_rts, __LINE__)(RenderTarget, DepthStencil)
#define GP_SCOPED_STATE(DeviceState) ::GP::DeviceStateScoped JOIN(_ds, __LINE__)(DeviceState)

namespace GP
{
	class GfxDeviceState;
	class GfxRenderTarget;

	class BeginRenderPassScoped
	{
	public:
		GP_DLL BeginRenderPassScoped(const std::string& debugName);
		GP_DLL ~BeginRenderPassScoped();
	};

	class DeviceStateScoped
	{
	public:
		GP_DLL DeviceStateScoped(GfxDeviceState* state);
		GP_DLL ~DeviceStateScoped();

	private:
		GfxDeviceState* m_LastState;
	};

	class RenderTargetScoped
	{
	public:
		GP_DLL RenderTargetScoped(GfxRenderTarget* rt, GfxRenderTarget* ds = nullptr);
		GP_DLL ~RenderTargetScoped();

	private:
		GfxRenderTarget* m_LastRT;
		GfxRenderTarget* m_LastDS;
	};
}