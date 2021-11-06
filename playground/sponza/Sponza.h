#pragma once

#include <GP.h>

#include "PlaygroundSample.h"

extern GP::Camera* g_Camera;

namespace SponzaSample
{
	class SponzaSample : public PlaygroundSample
	{
	public:
		~SponzaSample()
		{
			delete m_SkyboxTexture;
		}

		void SetupRenderer() override
		{
			std::string skybox_textures[] = {
			"playground/nature/resources/Sky/sky_R.png",
			"playground/nature/resources/Sky/sky_L.png",
			"playground/nature/resources/Sky/sky_U.png",
			"playground/nature/resources/Sky/sky_D.png",
			"playground/nature/resources/Sky/sky_B.png",
			"playground/nature/resources/Sky/sky_F.png",
			};
			m_SkyboxTexture = new GP::GfxCubemap(skybox_textures);

			GP::DefaultSceneRenderPass* sceneRenderPass = new GP::DefaultSceneRenderPass(g_Camera);
			sceneRenderPass->Load("playground/sponza/resources/GlassBunny/scene.gltf", Vec3(0.0f,50.0f,0.0f), VEC3_ONE * 500.0f);
			sceneRenderPass->Load("playground/sponza/resources/GlassBunny/scene.gltf", Vec3(0.0f,50.0f,50.0f), VEC3_ONE * 500.0f);
			sceneRenderPass->Load("playground/sponza/resources/sponza/sponza.gltf");

			GP::AddRenderPass(new GP::DefaultSkyboxRenderPass(g_Camera, m_SkyboxTexture));
			GP::AddRenderPass(sceneRenderPass);
		}

	private:
		GP::GfxCubemap* m_SkyboxTexture;
	};
}