#pragma once

#include <GP.h>

#include "PlaygroundSample.h"

extern GP::Camera* g_Camera;

namespace SponzaSample
{
	class SponzaSample : public PlaygroundSample
	{
	public:
		void SetupRenderer() override
		{
			GP::DefaultSceneRenderPass* sceneRenderPass = new GP::DefaultSceneRenderPass(g_Camera);
			sceneRenderPass->Load("playground/sponza/resources/sponza/sponza.gltf");
			GP::AddRenderPass(sceneRenderPass);
		}
	};
}