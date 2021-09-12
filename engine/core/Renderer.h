#pragma once

#include <vector>

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

		Scene* m_Scene = nullptr;
		std::vector<RenderPass*> m_Schedule;
	};
}