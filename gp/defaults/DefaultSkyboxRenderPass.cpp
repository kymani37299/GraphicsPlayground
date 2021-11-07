#include "DefaultSkyboxRenderPass.h"

#include "gfx/GfxTransformations.h"
#include "gfx/ScopedOperations.h"

namespace GP
{
	DefaultSkyboxRenderPass::~DefaultSkyboxRenderPass()
	{
		delete m_Shader;
	}

	void DefaultSkyboxRenderPass::Init(GfxDevice* device)
	{
		m_DeviceState.EnableBackfaceCulling(false);
		m_DeviceState.Compile();

		m_Shader = new GfxShader("gp/shaders/default_skybox.hlsl");
	}

	void DefaultSkyboxRenderPass::Render(GfxDevice* device)
	{
		RENDER_PASS("Default Skybox Render");

		DeviceStateScoped _dds(&m_DeviceState);

		device->BindShader(m_Shader);
		device->BindVertexBuffer(GfxDefaults::VB_CUBE);
		device->BindConstantBuffer(VS, m_Camera->GetBuffer(), 0);
		device->BindCubemap(PS, m_SkyboxCubemap, 0);
		device->Draw(GfxDefaults::VB_CUBE->GetNumVerts());

		device->UnbindTexture(PS, 0);
	}
}