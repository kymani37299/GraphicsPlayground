#include "SceneRenderer.h"

GP::GfxTexture* LoadTexture(const std::string& path);

void SceneRenderer::Init()
{
	m_ParamsBuffer = new GP::GfxConstantBuffer<CBSceneParams>();

	InitTerrain();
	InitSkybox();
}

void SceneRenderer::DestroyResources()
{
	delete m_ParamsBuffer;
	
	// Terrain
	delete m_TerrainShader;
	delete m_TerrainDeviceState;
	delete m_TerrainVB;
	delete m_TerrainIB;
	delete m_TerrainHeightMap;
	delete m_TerrainGrassTexture;

	// Skybox
	delete m_SkyboxShader;
	delete m_SkyboxDeviceState;
	delete m_SkyboxTexture;
}

void SceneRenderer::ReloadShaders()
{
	m_SkyboxShader->Reload();
	m_TerrainShader->Reload();
}

void SceneRenderer::DrawTerrain(GP::GfxDevice* device, GP::Camera* camera, CBSceneParams params)
{
	RENDER_PASS("SceneRenderer::DrawTerrain");

	if (!m_TerrainVB)
	{
		RENDER_PASS("SceneRenderer::GenerateTerrain");

		GP::GfxComputeShader terrainGenShader("playground/nature/shaders/terrain_generate.hlsl");
		m_TerrainVB = new GP::GfxStructuredBuffer<TerrainVert>(200 * 200, GP::BCF_UAV);

		device->BindShader(&terrainGenShader);
		device->BindRWStructuredBuffer(GP::CS, m_TerrainVB, 0);
		device->Dispatch(200, 200);
		device->BindRWStructuredBuffer(GP::CS, nullptr, 0);
	}

	GP::DeviceStateScoped dss(m_TerrainDeviceState);

	m_ParamsBuffer->Upload(params);
	
	device->BindShader(m_TerrainShader);
	device->BindVertexBuffer(m_TerrainIB);
	device->BindConstantBuffer(GP::VS, camera->GetBuffer(), 0);
	device->BindConstantBuffer(GP::VS, m_ParamsBuffer, 1);
	device->BindStructuredBuffer(GP::VS, m_TerrainVB, 2);
	device->BindTexture(GP::VS, m_TerrainHeightMap, 0);
	device->BindTexture(GP::PS, m_TerrainGrassTexture, 1);
	device->Draw(m_TerrainIB->GetNumVerts());

	device->UnbindTexture(GP::VS, 0);
	device->UnbindTexture(GP::PS, 1);
}

void SceneRenderer::DrawSkybox(GP::GfxDevice* device, GP::Camera* camera, CBSceneParams params)
{
	RENDER_PASS("SceneRenderer::DrawSkybox");

	GP::DeviceStateScoped _dss(m_SkyboxDeviceState);

	m_ParamsBuffer->Upload(params);

	device->BindShader(m_SkyboxShader);
	device->BindVertexBuffer(GP::GfxDefaults::VB_CUBE);
	device->BindConstantBuffer(GP::VS, camera->GetBuffer(), 0);
	device->BindConstantBuffer(GP::VS, m_ParamsBuffer, 1);
	device->BindTexture(GP::PS, m_SkyboxTexture, 0);
	device->Draw(GP::GfxDefaults::VB_CUBE->GetNumVerts());

	device->UnbindTexture(GP::PS, 0);
}

void SceneRenderer::InitTerrain()
{
	RENDER_PASS("SceneRenderer::InitTerrain");

	unsigned int TERRAIN_SIZE = 10000;
	unsigned int TERRAIN_SIDE_VERTS = 200;
	float TILE_SIZE = (float)TERRAIN_SIZE / TERRAIN_SIDE_VERTS;
	float GRASS_TEX_SIZE = 30.0;
	float TERRAIN_HEIGHT = -300.0;
	Vec2 TERRAIN_POSITION = Vec2(TERRAIN_SIZE, TERRAIN_SIZE) / 2.0f;

	std::vector<unsigned int> terrainIndices;
	terrainIndices.reserve((TERRAIN_SIDE_VERTS - 1) * (TERRAIN_SIDE_VERTS - 1) * 6);

	for (size_t i = 0; i < TERRAIN_SIDE_VERTS - 1; i++)
	{
		for (size_t j = 0; j < TERRAIN_SIDE_VERTS - 1; j++)
		{
			terrainIndices.push_back(i + TERRAIN_SIDE_VERTS * j);
			terrainIndices.push_back(i + 1 + TERRAIN_SIDE_VERTS * j);
			terrainIndices.push_back(i + TERRAIN_SIDE_VERTS * (j + 1));
			terrainIndices.push_back(i + TERRAIN_SIDE_VERTS * (j + 1));
			terrainIndices.push_back(i + TERRAIN_SIDE_VERTS * j + 1);
			terrainIndices.push_back(i + 1 + TERRAIN_SIDE_VERTS * (j + 1));
		}
	}

	m_TerrainIB = new GP::GfxVertexBuffer<unsigned int>(terrainIndices.data(), terrainIndices.size());
	m_TerrainShader = new GP::GfxShader("playground/nature/shaders/terrain.hlsl");

	m_TerrainDeviceState = new GP::GfxDeviceState();
	m_TerrainDeviceState->EnableDepthTest(true);
	m_TerrainDeviceState->Compile();

	m_TerrainHeightMap = LoadTexture("playground/nature/resources/HeightMap.png");
	m_TerrainGrassTexture = LoadTexture("playground/nature/resources/grass.png");
}

void SceneRenderer::InitSkybox()
{
	RENDER_PASS("SceneRenderer::InitSkybox");

	m_SkyboxShader = new GP::GfxShader("playground/nature/shaders/skybox.hlsl");

	m_SkyboxDeviceState = new GP::GfxDeviceState();
	m_SkyboxDeviceState->EnableBackfaceCulling(false);
	m_SkyboxDeviceState->Compile();

	GP::SceneLoading::CubemapData* skyboxData = GP::SceneLoading::LoadCubemap("playground/nature/resources/Sky/sky.png");
	GP::TextureDesc desc = {};
	desc.texData.push_back(skyboxData->pData[0]->pData);
	desc.texData.push_back(skyboxData->pData[1]->pData);
	desc.texData.push_back(skyboxData->pData[2]->pData);
	desc.texData.push_back(skyboxData->pData[3]->pData);
	desc.texData.push_back(skyboxData->pData[4]->pData);
	desc.texData.push_back(skyboxData->pData[5]->pData);
	desc.height = skyboxData->pData[0]->height;
	desc.width = skyboxData->pData[0]->width;
	desc.format = GP::TextureFormat::RGBA8_UNORM;
	desc.type = GP::TextureType::Cubemap;
	m_SkyboxTexture = new GP::GfxTexture(desc);
	GP::SceneLoading::FreeCubemap(skyboxData);
}