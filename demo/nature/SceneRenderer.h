#pragma once

#include <GP.h>
#include <memory>

namespace NatureSample
{
	struct CBSceneParams
	{
		Vec4 clipPlane = VEC4_ZERO;
		bool useClipping = false;
	};

	struct TerrainVert
	{
		Vec3 position;
		Vec2 uv;
	};

	class SceneRenderer
	{
	public:

		void Init(GP::GfxContext* context);
		void ReloadShaders();
		void DestroyResources();

		void DrawTerrain(GP::GfxContext* context, GP::Camera* camera, CBSceneParams params = CBSceneParams());
		void DrawSkybox(GP::GfxContext* context, GP::Camera* camera, CBSceneParams params = CBSceneParams());

	private:
		void InitTerrain(GP::GfxContext* context);
		void InitSkybox();

	private:
		GP::GfxConstantBuffer<CBSceneParams>* m_ParamsBuffer;

		// Terrain
		GP::GfxShader* m_TerrainShader;
		GP::GfxStructuredBuffer<TerrainVert>* m_TerrainVB;
		GP::GfxVertexBuffer<unsigned int>* m_TerrainIB;
		GP::GfxTexture2D* m_TerrainHeightMap;
		GP::GfxTexture2D* m_TerrainGrassTexture;

		// Skybox
		GP::GfxShader* m_SkyboxShader;
		GP::GfxCubemap* m_SkyboxTexture;
	};

	class ScenePass : public GP::RenderPass
	{
	public:
		ScenePass(SceneRenderer* sceneRenderer) :
			m_SceneRenderer(sceneRenderer)
		{ }

		inline virtual ~ScenePass()
		{
			m_SceneRenderer->DestroyResources();
		}

		inline virtual void Init(GP::GfxContext* context) override
		{
			m_SceneRenderer->Init(context);
		}

		inline virtual void Render(GP::GfxContext* context) override
		{

		}

		inline virtual void ReloadShaders() override
		{
			m_SceneRenderer->ReloadShaders();
		}

	private:
		SceneRenderer* m_SceneRenderer;
	};
}