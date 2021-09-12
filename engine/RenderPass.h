#pragma once

#define RENDER_PASS(device, debugName) BeginRenderPassScoped JOIN(brps,__LINE__)(device,debugName);

namespace GP
{
	class GfxDevice;

	class RenderPass
	{
	public:
		virtual ~RenderPass() {}

		virtual void Render(GfxDevice* device) = 0;
		virtual void ReloadShaders() = 0;
	};
}