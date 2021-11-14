#include "DefaultSceneRenderPass.h"

#include "gfx/GfxTransformations.h"
#include "gfx/GfxTexture.h"
#include "gfx/ScopedOperations.h"

namespace GP
{
	DefaultSceneRenderPass::~DefaultSceneRenderPass()
	{
		delete m_DiffuseSampler;
		delete m_ShaderOpaque;
		delete m_ShaderTransparent;
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

		m_ShaderOpaque = new GfxShader("gp/shaders/default_scene_phong.hlsl");
		m_ShaderTransparent = new GfxShader("gp/shaders/default_scene_phong.hlsl", { "USE_ALPHA_BLEND" });
		m_DiffuseSampler = new GfxSampler(SamplerFilter::Anisotropic, SamplerMode::Wrap);
	}

	void DefaultSceneRenderPass::Render(GfxDevice* device)
	{
		GP_SCOPED_PROFILE("Scene Default Render");

		{
			GP_SCOPED_PROFILE("Opaque");
			GP_SCOPED_STATE(&m_DeviceStateOpaque);

			device->BindShader(m_ShaderOpaque);
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
			GP_SCOPED_PROFILE("Transparent");
			GP_SCOPED_STATE(&m_DeviceStateTransparent);

			device->BindShader(m_ShaderTransparent);
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