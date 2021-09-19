#pragma once

#include "Common.h"

#include <vector>

namespace GP
{
	template<typename T> class GfxConstantBuffer;
	class RenderPass;
	class Scene;

	struct CBEngineGlobals
	{
		float screenWidth;
		float screenHeight;
		float time; // seconds
	};

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void InitRenderPasses();
		void Update(float dt);
		bool RenderIfShould();

		void ReloadShaders();

		inline void AddRenderPass(RenderPass* renderPass) { m_Schedule.push_back(renderPass); }
		inline GfxConstantBuffer<CBEngineGlobals>* GetGlobalsBuffer() const { return m_GlobalsBuffer; }

	private:
		void RenderFrame();

	private:
		bool m_ShouldRender = true;

		Scene* m_Scene = nullptr;
		std::vector<RenderPass*> m_Schedule;
		GfxConstantBuffer<CBEngineGlobals>* m_GlobalsBuffer;
	};
}