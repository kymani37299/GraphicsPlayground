#pragma once

#define RENDER_PASS(device, debugName) BeginRenderPassScoped JOIN(brps,__LINE__)(device,debugName);

class DxDevice;

class RenderPass
{
public:
	RenderPass(DxDevice& device): m_Device(device) {}
	virtual ~RenderPass() {}

	virtual void Render() = 0;

#ifdef DEBUG
	virtual void ReloadShaders() = 0;
#endif

protected:
	DxDevice& m_Device;
};