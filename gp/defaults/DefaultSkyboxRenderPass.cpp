#include "DefaultSkyboxRenderPass.h"

#include "gfx/GfxTransformations.h"
#include "gfx/ScopedOperations.h"

namespace GP
{
	DefaultSkyboxRenderPass::~DefaultSkyboxRenderPass()
	{
		delete m_Shader;
	}

	void DefaultSkyboxRenderPass::Init(GfxContext* context)
	{
		m_DeviceState.EnableBackfaceCulling(false);
		m_DeviceState.Compile();

		m_Shader = new GfxShader("gp/shaders/default_skybox.hlsl");
	}

	void DefaultSkyboxRenderPass::Render(GfxContext* context)
	{
		GP_SCOPED_PROFILE("Default Skybox Render");
		GP_SCOPED_STATE(&m_DeviceState);

		context->BindShader(m_Shader);
		context->BindVertexBuffer(GfxDefaults::VB_CUBE);
		context->BindConstantBuffer(VS, m_Camera->GetBuffer(), 0);
		context->BindCubemap(PS, m_SkyboxCubemap, 0);
		context->Draw(GfxDefaults::VB_CUBE->GetNumVerts());

		context->UnbindTexture(PS, 0);
	}
}