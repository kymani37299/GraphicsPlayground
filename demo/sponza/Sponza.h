#pragma once

#include <GP.h>

#include "DemoSample.h"

extern GP::Camera* g_Camera;

namespace SponzaSample
{
	class InstancedTestRenderPass : public GP::RenderPass
	{
	public:
		~InstancedTestRenderPass()
		{
			delete m_InstancePositions;
			delete m_InstancedShader;
			delete m_SceneInstancedShader;
		}

		void Init(GP::GfxContext*) override
		{
			m_InstancedScene.Load("demo/sponza/resources/GlassBunny/scene.gltf", VEC3_ZERO, VEC3_ONE * 500.0f);
			m_InstancePositions = new GP::GfxInstanceBuffer<Vec3>(10 * 10);
			m_InstancePositions->AddCreationFlags(GP::BCF_CPUWrite | GP::BCF_Usage_Dynamic);
			m_InstancePositions->Initialize();
			m_InstancedShader = new GP::GfxShader("demo/sponza/shaders/instanced_positions.hlsl");
			m_SceneInstancedShader = new GP::GfxShader("demo/sponza/shaders/instanced_positions.hlsl", { "USE_MODEL" });
			m_DeviceStateInstanced.EnableDepthTest(true);
			m_DeviceStateInstanced.EnableAlphaBlend(true);
			m_DeviceStateInstanced.EnableBackfaceCulling(true);
			m_DeviceStateInstanced.Compile();

			Vec3 instancePositions[100];
			for (size_t i = 0; i < 10; i++)
			{
				for (size_t j = 0; j < 10; j++)
				{
					const Vec3 position = Vec3(0.0f, 100.0f, 50.0f);
					instancePositions[i * 10 + j] = position + Vec3{ i * 50.0f, j * 50.0f, 50.0f };
				}
			}
			m_InstancePositions->Upload(instancePositions, sizeof(Vec3) * 100);
		}

		void Render(GP::GfxContext* context) override
		{
			GP_SCOPED_PROFILE("InstancedTest");
			GP_SCOPED_STATE(&m_DeviceStateInstanced);

			context->BindShader(m_InstancedShader);
			context->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(), 0);
			context->BindVertexBufferSlot(GP::GfxDefaults::VB_CUBE, 0);
			context->BindVertexBufferSlot(m_InstancePositions, 1);
			context->DrawInstanced(GP::GfxDefaults::VB_CUBE->GetNumVerts(), 10 * 10);

			context->BindShader(m_SceneInstancedShader);
			m_InstancedScene.ForEveryObject([context](const GP::SceneObject* sceneObject) {
				const GP::Mesh* mesh = sceneObject->GetMesh();
				context->BindConstantBuffer(GP::VS, sceneObject->GetTransformBuffer(), 1);
				context->BindVertexBufferSlot(mesh->GetPositionBuffer(), 0);
				context->BindIndexBuffer(mesh->GetIndexBuffer());
				context->DrawIndexedInstanced(mesh->GetIndexBuffer()->GetNumIndices(), 10 * 10);
				});
		}

		void ReloadShaders() override
		{
			m_InstancedShader->Reload();
			m_SceneInstancedShader->Reload();
		}

	private:
		GP::Scene m_InstancedScene;
		GP::GfxInstanceBuffer<Vec3>* m_InstancePositions;
		GP::GfxShader* m_InstancedShader;
		GP::GfxShader* m_SceneInstancedShader;
		GP::GfxDeviceState m_DeviceStateInstanced;
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

			GP::DefaultSceneRenderPass* sceneRenderPass = new GP::DefaultSceneRenderPass(g_Camera);
			sceneRenderPass->Load("demo/sponza/resources/GlassBunny/scene.gltf", Vec3(0.0f,50.0f,0.0f), VEC3_ONE * 500.0f, Vec3(0.0,0.0,1.57));
			sceneRenderPass->Load("demo/sponza/resources/GlassBunny/scene.gltf", Vec3(0.0f,50.0f,50.0f), VEC3_ONE * 500.0f, Vec3(0.0, 0.0, 1.57));
			sceneRenderPass->Load("demo/sponza/resources/sponza/sponza.gltf");

			GP::AddRenderPass(new GP::DefaultSkyboxRenderPass(g_Camera, m_SkyboxTexture));
			GP::AddRenderPass(sceneRenderPass);
			GP::AddRenderPass(new InstancedTestRenderPass());
		}

	private:
		GP::GfxCubemap* m_SkyboxTexture;
	};
}