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
		void GenerateTerrain(GP::GfxContext* context);

	private:
		GP::GfxConstantBuffer<CBSceneParams> m_ParamsBuffer;

		// Terrain
		GP::GfxShader m_TerrainShader{ "demo/nature/shaders/terrain.hlsl" };
		GP::GfxStructuredBuffer<TerrainVert> m_TerrainVB{ 200 * 200 };
		GP::GfxVertexBuffer<unsigned int>* m_TerrainIB;
		GP::GfxTexture2D m_TerrainHeightMap{ "demo/nature/resources/HeightMap.png" };
		GP::GfxTexture2D m_TerrainGrassTexture{ "demo/nature/resources/grass.png" };

		// Skybox
		std::string SKYBOX_TEXTURES[6] = {
			"demo/nature/resources/Sky/sky_R.png",
			"demo/nature/resources/Sky/sky_L.png",
			"demo/nature/resources/Sky/sky_U.png",
			"demo/nature/resources/Sky/sky_D.png",
			"demo/nature/resources/Sky/sky_B.png",
			"demo/nature/resources/Sky/sky_F.png",
		};

		GP::GfxShader m_SkyboxShader{ "demo/nature/shaders/skybox.hlsl" };
		GP::GfxCubemap m_SkyboxTexture{ SKYBOX_TEXTURES };
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