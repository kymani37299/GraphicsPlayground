#pragma once

#include <GP.h>

#include "DemoSample.h"

extern GP::Camera* g_Camera;

namespace SponzaSample
{
	GP::GfxRenderTarget* m_SceneRT;
	GP::GfxTexture2D* m_SceneDepth;

	class CelPass : public GP::RenderPass
	{
	public:
		virtual ~CelPass()
		{
			delete m_CelShader;
			delete m_AnisotropicWrap;
			delete m_SceneRT;
			delete m_SceneDepth;
		}

		virtual void Init(GP::GfxContext*) override
		{
			m_Scene.Load("demo/sponza/resources/sponza/sponza.gltf", VEC3_ZERO, VEC3_ONE * 1.5f);

			m_CelShader = new GP::GfxShader{ "demo/sponza/shaders/cel_shading.hlsl" };
			m_AnisotropicWrap = new GP::GfxSampler(GP::SamplerFilter::Anisotropic, GP::SamplerMode::Wrap);

			m_CelDeviceState.EnableDepthTest(true);
			m_CelDeviceState.Compile();
		}

		virtual void Render(GP::GfxContext* context) override
		{
			GP_SCOPED_PROFILE("Cel Shading");
			GP_SCOPED_STATE(&m_CelDeviceState);
			GP_SCOPED_RT(m_SceneRT, m_SceneRT);

			context->Clear();
			context->BindShader(m_CelShader);
			context->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(), 0);
			context->BindSampler(GP::PS, m_AnisotropicWrap, 0);
			m_Scene.ForEveryObject([&context](GP::SceneObject* sceneObejct) {

				// Mesh
				const GP::Mesh* mesh = sceneObejct->GetMesh();
				context->BindConstantBuffer(GP::VS, sceneObejct->GetTransformBuffer(), 1);
				context->BindVertexBufferSlot(mesh->GetPositionBuffer(), 0);
				context->BindVertexBufferSlot(mesh->GetNormalBuffer(), 1);
				context->BindVertexBufferSlot(mesh->GetUVBuffer(), 2);
				context->BindIndexBuffer(mesh->GetIndexBuffer());

				// Material
				const GP::Material* material = sceneObejct->GetMaterial();
				context->BindTexture2D(GP::PS, material->GetDiffuseTexture(), 0);

				context->DrawIndexed(mesh->GetIndexBuffer()->GetNumIndices());
				});
		}

		virtual void ReloadShaders() override
		{
			m_CelShader->Reload();
		}

	private:
		GP::Scene m_Scene;
		GP::GfxShader* m_CelShader;
		GP::GfxSampler* m_AnisotropicWrap;
		GP::GfxDeviceState m_CelDeviceState;
	};

	class SponzaSample : public DemoSample
	{
	public:
		~SponzaSample()
		{
			delete m_SkyboxTexture;
		}

		void SetupRenderer() override
		{
			std::string skybox_textures[] = {
			"demo/nature/resources/Sky/sky_R.png",
			"demo/nature/resources/Sky/sky_L.png",
			"demo/nature/resources/Sky/sky_U.png",
			"demo/nature/resources/Sky/sky_D.png",
			"demo/nature/resources/Sky/sky_B.png",
			"demo/nature/resources/Sky/sky_F.png",
			};
			m_SkyboxTexture = new GP::GfxCubemap(skybox_textures);

			m_SceneRT = new GP::GfxRenderTarget(WINDOW_WIDTH, WINDOW_HEIGHT, 1, true);
			m_SceneDepth = new GP::GfxTexture2D(m_SceneRT->GetDepthResrouce());

			GP::AddRenderPass(new GP::DefaultSkyboxRenderPass(g_Camera, m_SkyboxTexture));
			GP::AddRenderPass(new CelPass());
			GP::AddRenderPass(new GP::CopyToRTPass(m_SceneDepth, nullptr));
		}

	private:
		GP::GfxCubemap* m_SkyboxTexture;
	};
}