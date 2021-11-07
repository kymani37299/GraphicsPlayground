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
		m_DeviceStateOpaque.EnableDepthTest(true);
		m_DeviceStateOpaque.EnableBackfaceCulling(true);
		m_DeviceStateOpaque.Compile();

		m_DeviceStateTransparent.EnableDepthTest(true);
		m_DeviceStateTransparent.EnableAlphaBlend(true);
		m_DeviceStateTransparent.EnableBackfaceCulling(true);
		m_DeviceStateTransparent.Compile();

		m_Shader = new GfxShader("gp/shaders/default_scene_phong.hlsl");
		m_DiffuseSampler = new GfxSampler(SamplerFilter::Anisotropic, SamplerMode::Wrap);
	}

	void DefaultSceneRenderPass::Render(GfxDevice* device)
	{
		RENDER_PASS("Scene Default Render");

		{
			RENDER_PASS("Opaque");
			DeviceStateScoped _dds(&m_DeviceStateOpaque);
			device->BindShader(m_Shader);
			device->BindConstantBuffer(VS, m_Camera->GetBuffer(), 0);
			device->BindSampler(PS, m_DiffuseSampler, 0);
			m_Scene.ForEveryOpaqueObject([device](const SceneObject* sceneObject) {
				const Mesh* mesh = sceneObject->GetMesh();
				device->BindConstantBuffer(VS, sceneObject->GetTransformBuffer(), 1);
				device->BindVertexBufferSlot(mesh->GetPositionBuffer(), 0);
				device->BindVertexBufferSlot(mesh->GetUVBuffer(), 1);
				device->BindVertexBufferSlot(mesh->GetNormalBuffer(), 2);
				device->BindVertexBufferSlot(mesh->GetTangentBuffer(), 3);
				device->BindIndexBuffer(mesh->GetIndexBuffer());
				device->BindTexture2D(PS, sceneObject->GetMaterial()->GetDiffuseTexture(), 0);
				device->DrawIndexed(mesh->GetIndexBuffer()->GetNumIndices());
				});
			device->UnbindTexture(PS, 0);
		}

		{
			RENDER_PASS("Transparent");
			DeviceStateScoped _dds(&m_DeviceStateTransparent);
			device->BindShader(m_Shader);
			device->BindConstantBuffer(VS, m_Camera->GetBuffer(), 0);
			device->BindSampler(PS, m_DiffuseSampler, 0);
			m_Scene.ForEveryTransparentObjectSorted(m_Camera->GetPosition(), [device](const SceneObject* sceneObject) {
				const Mesh* mesh = sceneObject->GetMesh();
				device->BindConstantBuffer(VS, sceneObject->GetTransformBuffer(), 1);
				device->BindVertexBufferSlot(mesh->GetPositionBuffer(), 0);
				device->BindVertexBufferSlot(mesh->GetUVBuffer(), 1);
				device->BindVertexBufferSlot(mesh->GetNormalBuffer(), 2);
				device->BindVertexBufferSlot(mesh->GetTangentBuffer(), 3);
				device->BindIndexBuffer(mesh->GetIndexBuffer());
				device->BindTexture2D(PS, sceneObject->GetMaterial()->GetDiffuseTexture(), 0);
				device->DrawIndexed(mesh->GetIndexBuffer()->GetNumIndices());
				});
			device->UnbindTexture(PS, 0);
		}
	}
}