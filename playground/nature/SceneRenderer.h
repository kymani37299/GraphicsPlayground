#pragma once

#include <Engine.h>
#include <memory>

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

	void Init();
	void ReloadShaders();
	void DestroyResources();

	void DrawTerrain(GP::GfxDevice* device, GP::Camera* camera, CBSceneParams params = CBSceneParams());
	void DrawSkybox(GP::GfxDevice* device, GP::Camera* camera, CBSceneParams params = CBSceneParams());

private:
	void InitTerrain();
	void InitSkybox();

private:
	GP::GfxConstantBuffer<CBSceneParams>* m_ParamsBuffer;

	// Terrain
	GP::GfxShader* m_TerrainShader;
	GP::GfxDeviceState* m_TerrainDeviceState;
	GP::GfxVertexBuffer* m_TerrainVB;
	GP::GfxIndexBuffer* m_TerrainIB;
	GP::GfxTexture* m_TerrainHeightMap;
	GP::GfxTexture* m_TerrainGrassTexture;

	// Skybox
	GP::GfxShader* m_SkyboxShader;
	GP::GfxDeviceState* m_SkyboxDeviceState;
	GP::GfxTexture* m_SkyboxTexture;
};

class ScenePass : public GP::RenderPass
{
public:
	ScenePass(SceneRenderer* sceneRenderer):
		m_SceneRenderer(sceneRenderer)
	{ }

	inline virtual ~ScenePass()
	{
		m_SceneRenderer->DestroyResources();
	}

	inline virtual void Init() override
	{
		m_SceneRenderer->Init();
	}

	inline virtual void Render(GP::GfxDevice* device) override
	{

	}

	inline virtual void ReloadShaders() override
	{
		m_SceneRenderer->ReloadShaders();
	}

private:
	SceneRenderer* m_SceneRenderer;
};