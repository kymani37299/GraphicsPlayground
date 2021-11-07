#pragma once

#include <GP.h>

#include "DemoSample.h"

extern GP::Camera* g_Camera;

namespace SponzaSample
{
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
		}

	private:
		GP::GfxCubemap* m_SkyboxTexture;
	};
}