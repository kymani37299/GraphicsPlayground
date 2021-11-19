#pragma once

#include "core/RenderPass.h"
#include "gfx/GfxDevice.h"
#include "gfx/GfxShader.h"

namespace GP
{
	class Camera;

	class DefaultSkyboxRenderPass : public RenderPass
	{
	public:
		DefaultSkyboxRenderPass(Camera* camera, GfxCubemap* skyboxCubemap) :
			m_Camera(camera),
			m_SkyboxCubemap(skyboxCubemap) {}

		GP_DLL ~DefaultSkyboxRenderPass();

		GP_DLL virtual void Init(GfxContext* context) override;
		GP_DLL virtual void Render(GfxContext* context) override;

		inline virtual void ReloadShaders() override
		{
			m_Shader->Reload();
		}

	private:
		Camera* m_Camera = nullptr;
		GfxDeviceState m_DeviceState;
		GfxShader* m_Shader = nullptr;
		GfxCubemap* m_SkyboxCubemap = nullptr;
	};
}