#pragma once

#include "Common.h"

#define RENDER_PASS(debugName) ::GP::BeginRenderPassScoped JOIN(brps,__LINE__)(debugName)

namespace GP
{
	class GfxDevice;

	class RenderPass
	{
	public:
		virtual ~RenderPass() {}

		virtual void Init() = 0;
		virtual void Render(GfxDevice* device) = 0;
		virtual void ReloadShaders() = 0;
	};
}