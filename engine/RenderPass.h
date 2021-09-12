#pragma once

#define RENDER_PASS(device, debugName) BeginRenderPassScoped JOIN(brps,__LINE__)(device,debugName);

namespace GP
{
	class GfxDevice;

	class RenderPass
	{
	public:
		RenderPass(GfxDevice& device) : m_Device(device) {}
		virtual ~RenderPass() {}

		virtual void Render() = 0;
		virtual void ReloadShaders() = 0;

	protected:
		GfxDevice& m_Device;
	};
}