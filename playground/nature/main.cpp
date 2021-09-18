#include <Engine.h>

#include <memory>

using namespace std;

#define RUN_NATURE_SAMPLE

GP::Camera* g_Camera = nullptr;

class SkyboxPass : public GP::RenderPass
{
public:

	virtual void Init() override 
	{
		m_DeviceState.reset(new GP::GfxDeviceState());
		m_DeviceState->EnableBackfaceCulling(false);
		m_DeviceState->Compile();

		m_Shader.reset(new GP::GfxShader("playground/nature/skybox.hlsl"));

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
		m_SkyboxTexture.reset(new GP::GfxTexture(desc));
		GP::SceneLoading::FreeCubemap(skyboxData);
	}

	virtual void Render(GP::GfxDevice* device) override 
	{
		RENDER_PASS("Skybox");

		GP::DeviceStateScoped _dss(m_DeviceState.get());
		device->BindShader(m_Shader.get());
		device->BindVertexBuffer(GP::GfxDefaults::VB_CUBE);
		device->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(), 0);
		device->BindTexture(GP::PS, m_SkyboxTexture.get(), 0);
		device->Draw(GP::GfxDefaults::VB_CUBE->GetNumVerts());
		device->UnbindTexture(GP::PS, 0);
	}

	virtual void ReloadShaders() override 
	{
		m_Shader->Reload();
	}

private:
	unique_ptr<GP::GfxDeviceState> m_DeviceState;
	unique_ptr<GP::GfxShader> m_Shader;
	unique_ptr<GP::GfxTexture> m_SkyboxTexture;
};

class TerrainPass : public GP::RenderPass
{
	struct TerrainVert
	{
		Vec3 position;
		Vec2 uv;
	};

public:

	virtual void Init() override
	{
		unsigned int TERRAIN_SIZE = 10000;
		unsigned int TERRAIN_SIDE_VERTS = 20;
		float TILE_SIZE = (float)TERRAIN_SIZE / TERRAIN_SIDE_VERTS;
		float GRASS_TEX_SIZE = 30.0;
		float TERRAIN_HEIGHT = -300.0;
		Vec2 TERRAIN_POSITION = Vec2(TERRAIN_SIZE,TERRAIN_SIZE) / 2.0f;

		std::vector<TerrainVert> terrainVerts;
		std::vector<unsigned int> terrainIndices;

		terrainVerts.reserve(TERRAIN_SIDE_VERTS * TERRAIN_SIDE_VERTS);
		terrainIndices.reserve((TERRAIN_SIDE_VERTS - 1) * (TERRAIN_SIDE_VERTS - 1) * 6);
		for (size_t i = 0; i < TERRAIN_SIDE_VERTS; i++)
		{
			for (size_t j = 0; j < TERRAIN_SIDE_VERTS; j++)
			{
				Vec2 modelPos = TILE_SIZE * Vec2(i, j);
				Vec2 pos2D = TERRAIN_POSITION - modelPos;

				TerrainVert terrainVert;
				terrainVert.position = Vec3(pos2D.x, TERRAIN_HEIGHT, pos2D.y);
				terrainVert.uv = glm::fract(modelPos / (float)GRASS_TEX_SIZE);

				terrainVerts.push_back(terrainVert);
			}
		}

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

		GP::VertexBufferData vertexData = {};
		vertexData.numBytes = terrainVerts.size() * sizeof(TerrainVert);
		vertexData.stride = sizeof(TerrainVert);
		vertexData.pData = terrainVerts.data();
		m_TerrainVB.reset(new GP::GfxVertexBuffer(vertexData));
		m_TerrainIB.reset(new GP::GfxIndexBuffer(terrainIndices.data(), terrainIndices.size()));

		m_DeviceState.reset(new GP::GfxDeviceState());
		m_DeviceState->EnableDepthTest(true);
		m_DeviceState->Compile();

		m_Shader.reset(new GP::GfxShader("playground/nature/terrain.hlsl"));

		GP::SceneLoading::TextureData* heightmapData = GP::SceneLoading::LoadTexture("playground/nature/resources/PerlinNoise.png");
		GP::TextureDesc heightmapDesc = {};
		heightmapDesc.height = heightmapData->height;
		heightmapDesc.width = heightmapData->width;
		heightmapDesc.texData.push_back(heightmapData->pData);
		heightmapDesc.type = GP::TextureType::Texture2D;
		heightmapDesc.format = GP::TextureFormat::RGBA8_UNORM;
		m_HeightMap.reset(new GP::GfxTexture(heightmapDesc));
		GP::SceneLoading::FreeTexture(heightmapData);

		GP::SceneLoading::TextureData* grassData = GP::SceneLoading::LoadTexture("playground/nature/resources/grass.png");
		GP::TextureDesc grassDesc = {};
		grassDesc.height = grassData->height;
		grassDesc.width = grassData->width;
		grassDesc.texData.push_back(grassData->pData);
		grassDesc.type = GP::TextureType::Texture2D;
		grassDesc.format = GP::TextureFormat::RGBA8_UNORM;
		m_GrassTexture.reset(new GP::GfxTexture(grassDesc));
		GP::SceneLoading::FreeTexture(grassData);
	}

	virtual void Render(GP::GfxDevice* device) override
	{
		RENDER_PASS("Terrain");

		GP::DeviceStateScoped dss(m_DeviceState.get());
		GP::RenderTargetScoped rts(device->GetFinalRT(), device->GetFinalRT());
		RenderTerrain(device, m_Shader.get());
	}

	virtual void ReloadShaders() override
	{
		m_Shader->Reload();
	}

public:
	void RenderTerrain(GP::GfxDevice* device, GP::GfxShader* shader)
	{
		device->BindShader(shader);
		device->BindVertexBuffer(m_TerrainVB.get());
		device->BindIndexBuffer(m_TerrainIB.get());
		device->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(), 0);
		device->BindTexture(GP::VS, m_HeightMap.get(), 0);
		device->BindTexture(GP::PS, m_GrassTexture.get(), 1);
		device->DrawIndexed(m_TerrainIB->GetNumIndices());

		device->UnbindTexture(GP::VS, 0);
		device->UnbindTexture(GP::PS, 1);
	}

private:
	unique_ptr<GP::GfxVertexBuffer> m_TerrainVB;
	unique_ptr<GP::GfxIndexBuffer> m_TerrainIB;

	unique_ptr<GP::GfxShader> m_Shader;
	unique_ptr<GP::GfxDeviceState> m_DeviceState;

	unique_ptr<GP::GfxTexture> m_HeightMap;
	unique_ptr<GP::GfxTexture> m_GrassTexture;
};

class WaterPass : public GP::RenderPass
{
	const float WATER_REF_RESOLUTION = WINDOW_WIDTH/2.0f;
	const float WATER_HEIGHT = -50.0f;
	const float WATER_HEIGHT_BIAS = 5.0; // Used to remove aliasing when water is slicing terrain

public:

	WaterPass(TerrainPass* terrainPass, SkyboxPass* skyboxPass):
		m_TerrainPass(terrainPass),
		m_SkyboxPass(skyboxPass)
	{ }

	virtual void Init() override
	{
		m_GeometryShader.reset(new GP::GfxShader("playground/nature/water_reflection_refraction.hlsl"));
		m_WaterShader.reset(new GP::GfxShader("playground/nature/water.hlsl"));
		m_PlaneModel.reset(new GP::ModelTransform());
		m_PlaneModel->SetPosition(Vec3(0.0f, WATER_HEIGHT, 0.0f));
		m_PlaneModel->SetScale(10000.0f * VEC3_ONE);

		m_DeviceState.reset(new GP::GfxDeviceState());
		m_DeviceState->EnableDepthTest(true);
		m_DeviceState->EnableBackfaceCulling(false);
		m_DeviceState->Compile();

		m_WaterHeightBuffer.reset(new GP::GfxConstantBuffer<CBGeometryParams>());

		GP::RenderTargetDesc rtDesc = {};
		rtDesc.height = (unsigned int) WATER_REF_RESOLUTION * ASPECT_RATIO;
		rtDesc.width = (unsigned int) WATER_REF_RESOLUTION;
		rtDesc.useDepth = true;
		rtDesc.useStencil = false;
		rtDesc.numRTs = 1;

		m_WaterRefraction.reset(new GP::GfxRenderTarget(rtDesc));
		m_WaterReflection.reset(new GP::GfxRenderTarget(rtDesc));
	}

	virtual void Render(GP::GfxDevice* device) override
	{
		RENDER_PASS("Water");

		GP::DeviceStateScoped _dss(m_DeviceState.get());
		
		RenderRefractionTexture(device);
		RenderReflectionTexture(device);

		{
			RENDER_PASS("Water plane");

			device->BindShader(m_WaterShader.get());
			device->BindVertexBuffer(GP::GfxDefaults::VB_QUAD);
			device->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(), 0);
			device->BindConstantBuffer(GP::VS, m_PlaneModel->GetBuffer(), 1);
			device->BindTexture(GP::PS, m_WaterReflection.get(), 0);
			device->BindTexture(GP::PS, m_WaterRefraction.get(), 1);
			device->Draw(GP::GfxDefaults::VB_QUAD->GetNumVerts());

			device->UnbindTexture(GP::PS, 0);
			device->UnbindTexture(GP::PS, 1);
		}

	}

	virtual void ReloadShaders() override
	{
		m_GeometryShader->Reload();
		m_WaterShader->Reload();
	}

private:
	void RenderReflectionTexture(GP::GfxDevice* device)
	{
		RENDER_PASS("ReflectionTexture");

		GP::RenderTargetScoped _rts(m_WaterReflection.get(), m_WaterReflection.get());
		device->Clear();

		CBGeometryParams params = {};
		params.isRefraction = false;
		params.waterHeight = WATER_HEIGHT + WATER_HEIGHT_BIAS;
		m_WaterHeightBuffer->Upload(params);

		// Setup camera under the water - Maybe do this in some other camera buffer#
		Vec3 camPos = g_Camera->GetPosition();
		float playerWaterHeight = camPos.y - WATER_HEIGHT;
		camPos.y -= 2.0f * playerWaterHeight;
		Vec3 camRot = g_Camera->GetRotation();
		camRot.x = -camRot.x;
		g_Camera->SetPosition(camPos);
		g_Camera->SetRotation(camRot);

		device->BindConstantBuffer(GP::VS, m_WaterHeightBuffer.get(), 2);
		m_SkyboxPass->Render(device);
		m_TerrainPass->RenderTerrain(device, m_GeometryShader.get());

		// Revert camera changes
		camPos.y += 2.0f * playerWaterHeight;
		camRot.x = -camRot.x;
		g_Camera->SetPosition(camPos);
		g_Camera->SetRotation(camRot);
	}

	void RenderRefractionTexture(GP::GfxDevice* device)
	{
		RENDER_PASS("RefractionTexture");

		GP::RenderTargetScoped _rts(m_WaterRefraction.get(), m_WaterRefraction.get());
		device->Clear();

		CBGeometryParams params = {};
		params.isRefraction = true;
		params.waterHeight = WATER_HEIGHT + WATER_HEIGHT_BIAS;
		m_WaterHeightBuffer->Upload(params);

		device->BindConstantBuffer(GP::VS, m_WaterHeightBuffer.get(), 2);
		m_TerrainPass->RenderTerrain(device, m_GeometryShader.get());
	}

private:
	struct CBGeometryParams
	{
		alignas(16) float waterHeight;
		bool isRefraction;
	};

private:
	TerrainPass* m_TerrainPass;
	SkyboxPass* m_SkyboxPass;

	unique_ptr<GP::GfxShader> m_GeometryShader;
	unique_ptr<GP::GfxShader> m_WaterShader;
	unique_ptr<GP::ModelTransform> m_PlaneModel;
	unique_ptr<GP::GfxDeviceState> m_DeviceState;

	unique_ptr<GP::GfxRenderTarget> m_WaterReflection;
	unique_ptr<GP::GfxRenderTarget> m_WaterRefraction;

	unique_ptr<GP::GfxConstantBuffer<CBGeometryParams>> m_WaterHeightBuffer;
};

#ifdef RUN_NATURE_SAMPLE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	GP::Init(hInstance);
	
	GP::Camera playerCamera;
	g_Camera = &playerCamera;
	g_Camera->SetPosition({ -30.0,50.0,0.0 });

	TerrainPass* terrainPass = new TerrainPass();
	SkyboxPass* skyboxPass = new SkyboxPass();

	GP::ShowCursor(false);
	GP::SetDefaultController(g_Camera);
	GP::AddRenderPass(skyboxPass);
	GP::AddRenderPass(new WaterPass(terrainPass, skyboxPass));
	GP::AddRenderPass(terrainPass);
	GP::Run();
	GP::Deinit();
}
#endif // RUN_NATURE_SAMPLE