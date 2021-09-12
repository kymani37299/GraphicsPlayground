#pragma once

#include <vector>

#include "GfxCore.h"

namespace GP
{
	class RenderPass;
	class Window;
	class Scene;

	class Renderer
	{
	public:
		Renderer(Window* window);
		~Renderer();

		void Update(float dt);
		bool RenderIfShould();

		void ReloadShaders();

	private:
		void RenderFrame();

	private:
		bool m_ShouldRender = true;

		GfxDevice m_Device;
		Scene* m_Scene = nullptr;
		std::vector<RenderPass*> m_Schedule;
	};
}