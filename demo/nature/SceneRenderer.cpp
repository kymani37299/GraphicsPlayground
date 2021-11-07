#include "SceneRenderer.h"

namespace NatureSample
{
	void SceneRenderer::Init(GP::GfxDevice* device)
	{
		m_ParamsBuffer = new GP::GfxConstantBuffer<CBSceneParams>();

		InitTerrain(device);
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

		GP::DeviceStateScoped dss(m_TerrainDeviceState);

		m_ParamsBuffer->Upload(params);

		device->BindShader(m_TerrainShader);
		device->BindVertexBuffer(m_TerrainIB);
		device->BindConstantBuffer(GP::VS, camera->GetBuffer(), 0);
		device->BindConstantBuffer(GP::VS, m_ParamsBuffer, 1);
		device->BindStructuredBuffer(GP::VS, m_TerrainVB, 2);
		device->BindTexture2D(GP::VS, m_TerrainHeightMap, 0);
		device->BindTexture2D(GP::PS, m_TerrainGrassTexture, 1);
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
		device->BindCubemap(GP::PS, m_SkyboxTexture, 0);
		device->Draw(GP::GfxDefaults::VB_CUBE->GetNumVerts());

		device->UnbindTexture(GP::PS, 0);
	}

	struct TerrainCreateInfo
	{
		unsigned int terrainSize;
		unsigned int terrainSideVerts;
		float terrainHeight;
		char _padding[2];
		Vec2 terrainPosition;
	};

	void SceneRenderer::InitTerrain(GP::GfxDevice* device)
	{
		RENDER_PASS("SceneRenderer::InitTerrain");

		TerrainCreateInfo terrainInfo = {};
		terrainInfo.terrainSize = 10000;
		terrainInfo.terrainSideVerts = 200;
		terrainInfo.terrainHeight = -300.0f;
		terrainInfo.terrainPosition = Vec2(terrainInfo.terrainSize, terrainInfo.terrainSize) / 2.0f;

		// Terrain vertices
		{
			// TODO: Copy structured buffer to vertex buffer

			GP::GfxConstantBuffer<TerrainCreateInfo> cbCreateInfo;
			cbCreateInfo.Upload(terrainInfo);

			GP::GfxComputeShader terrainGenShader("demo/nature/shaders/terrain_generate.hlsl");
			m_TerrainVB = new GP::GfxStructuredBuffer<TerrainVert>(200 * 200, GP::BCF_UAV);

			device->BindShader(&terrainGenShader);
			device->BindConstantBuffer(GP::CS, &cbCreateInfo, 0);
			device->BindRWStructuredBuffer(GP::CS, m_TerrainVB, 0);
			device->Dispatch(200, 200);
			device->BindRWStructuredBuffer(GP::CS, nullptr, 0);
		}

		// Terrain indices
		{
			
			std::vector<unsigned int> terrainIndices;
			terrainIndices.reserve((terrainInfo.terrainSideVerts - 1) * (terrainInfo.terrainSideVerts - 1) * 6);

			for (size_t i = 0; i < terrainInfo.terrainSideVerts - 1; i++)
			{
				for (size_t j = 0; j < terrainInfo.terrainSideVerts - 1; j++)
				{
					terrainIndices.push_back(i + terrainInfo.terrainSideVerts * j);
					terrainIndices.push_back(i + 1 + terrainInfo.terrainSideVerts * j);
					terrainIndices.push_back(i + terrainInfo.terrainSideVerts * (j + 1));
					terrainIndices.push_back(i + terrainInfo.terrainSideVerts * (j + 1));
					terrainIndices.push_back(i + terrainInfo.terrainSideVerts * j + 1);
					terrainIndices.push_back(i + 1 + terrainInfo.terrainSideVerts * (j + 1));
				}
			}

			m_TerrainIB = new GP::GfxVertexBuffer<unsigned int>(terrainIndices.data(), terrainIndices.size());
		}
		
		m_TerrainShader = new GP::GfxShader("demo/nature/shaders/terrain.hlsl");

		m_TerrainDeviceState = new GP::GfxDeviceState();
		m_TerrainDeviceState->EnableDepthTest(true);
		m_TerrainDeviceState->Compile();

		m_TerrainHeightMap = new GP::GfxTexture2D("demo/nature/resources/HeightMap.png");
		m_TerrainGrassTexture = new GP::GfxTexture2D("demo/nature/resources/grass.png");
	}

	void SceneRenderer::InitSkybox()
	{
		RENDER_PASS("SceneRenderer::InitSkybox");

		m_SkyboxShader = new GP::GfxShader("demo/nature/shaders/skybox.hlsl");

		m_SkyboxDeviceState = new GP::GfxDeviceState();
		m_SkyboxDeviceState->EnableBackfaceCulling(false);
		m_SkyboxDeviceState->Compile();

		std::string skybox_textures[] = {
			"demo/nature/resources/Sky/sky_R.png",
			"demo/nature/resources/Sky/sky_L.png",
			"demo/nature/resources/Sky/sky_U.png",
			"demo/nature/resources/Sky/sky_D.png",
			"demo/nature/resources/Sky/sky_B.png",
			"demo/nature/resources/Sky/sky_F.png",
		};
		m_SkyboxTexture = new GP::GfxCubemap(skybox_textures);
	}
}