#include "DefaultSceneRenderPass.h"

#include "gfx/GfxTransformations.h"

namespace GP
{
	DefaultSceneRenderPass::~DefaultSceneRenderPass()
	{
		delete m_DiffuseSampler;
		delete m_Shader;
	}

	void DefaultSceneRenderPass::Init(GfxDevice* device)
	{
		m_DeviceState.EnableDepthTest(true);
		//m_DeviceState.EnableAlphaBlend(true);
		m_DeviceState.Compile();

		m_Shader = new GfxShader("engine/shaders/default_scene_phong.hlsl");
		m_DiffuseSampler = new GfxSampler(SamplerFilter::Anisotropic, SamplerMode::Wrap);
	}

	void DefaultSceneRenderPass::Render(GfxDevice* device)
	{
		RENDER_PASS("Scene Default Render");

		DeviceStateScoped _dds(&m_DeviceState);
		device->BindShader(m_Shader);
		device->BindConstantBuffer(VS, m_Camera->GetBuffer(), 0);
		device->BindSampler(PS, m_DiffuseSampler, 0);
		m_Scene.ForEverySceneObject([device](const SceneObject* sceneObject) {
			const Mesh* mesh = sceneObject->GetMesh();
			device->BindConstantBuffer(VS, sceneObject->GetInstanceBuffer(), 1);
			device->BindVertexBufferSlot(mesh->GetPositionBuffer(), 0);
			device->BindVertexBufferSlot(mesh->GetUVBuffer(), 1);
			device->BindVertexBufferSlot(mesh->GetNormalBuffer(), 2);
			device->BindVertexBufferSlot(mesh->GetTangentBuffer(), 3);
			device->BindIndexBuffer(mesh->GetIndexBuffer());
			device->BindTexture2D(PS, sceneObject->GetMaterial()->GetDiffuseTexture(), 0);
			device->DrawIndexed(mesh->GetIndexBuffer()->GetNumIndices());
			});
	}
}