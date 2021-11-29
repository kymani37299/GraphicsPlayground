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

	void DefaultSceneRenderPass::Init(GfxContext* context)
	{
		m_ShaderOpaque = new GfxShader("gp/shaders/default_scene_phong.hlsl");
		m_ShaderTransparent = new GfxShader("gp/shaders/default_scene_phong.hlsl", { "USE_ALPHA_BLEND" });
		m_DiffuseSampler = new GfxSampler(SamplerFilter::Anisotropic, SamplerMode::Wrap);
	}

	void DefaultSceneRenderPass::Render(GfxContext* context)
	{
		GP_SCOPED_PROFILE("Scene Default Render");

		{
			GP_SCOPED_PROFILE("Opaque");

			context->BindShader(m_ShaderOpaque);
			context->BindConstantBuffer(VS, m_Camera->GetBuffer(), 0);
			context->BindSampler(PS, m_DiffuseSampler, 0);
			m_Scene.ForEveryOpaqueObject([context](const SceneObject* sceneObject) {
				const Mesh* mesh = sceneObject->GetMesh();
				context->BindConstantBuffer(VS, sceneObject->GetTransformBuffer(), 1);
				context->BindVertexBufferSlot(mesh->GetPositionBuffer(), 0);
				context->BindVertexBufferSlot(mesh->GetUVBuffer(), 1);
				context->BindVertexBufferSlot(mesh->GetNormalBuffer(), 2);
				context->BindVertexBufferSlot(mesh->GetTangentBuffer(), 3);
				context->BindIndexBuffer(mesh->GetIndexBuffer());
				context->BindTexture2D(PS, sceneObject->GetMaterial()->GetDiffuseTexture(), 0);
				context->DrawIndexed(mesh->GetIndexBuffer()->GetNumIndices());
				});
			context->UnbindTexture(PS, 0);
		}

		{
			GP_SCOPED_PROFILE("Transparent");

			context->BindShader(m_ShaderTransparent);
			context->BindConstantBuffer(VS, m_Camera->GetBuffer(), 0);
			context->BindSampler(PS, m_DiffuseSampler, 0);
			m_Scene.ForEveryTransparentObjectSorted(m_Camera->GetPosition(), [context](const SceneObject* sceneObject) {
				const Mesh* mesh = sceneObject->GetMesh();
				context->BindConstantBuffer(VS, sceneObject->GetTransformBuffer(), 1);
				context->BindVertexBufferSlot(mesh->GetPositionBuffer(), 0);
				context->BindVertexBufferSlot(mesh->GetUVBuffer(), 1);
				context->BindVertexBufferSlot(mesh->GetNormalBuffer(), 2);
				context->BindVertexBufferSlot(mesh->GetTangentBuffer(), 3);
				context->BindIndexBuffer(mesh->GetIndexBuffer());
				context->BindTexture2D(PS, sceneObject->GetMaterial()->GetDiffuseTexture(), 0);
				context->DrawIndexed(mesh->GetIndexBuffer()->GetNumIndices());
				});
			context->UnbindTexture(PS, 0);
		}
	}
}