#pragma once

#include "gfx/GfxCommon.h"

#include <string>

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