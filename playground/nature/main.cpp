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

		device->BindShader(m_Shader.get());
		device->BindVertexBuffer(m_TerrainVB.get());
		device->BindIndexBuffer(m_TerrainIB.get());
		device->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(), 0);
		device->BindTexture(GP::VS, m_HeightMap.get(), 0);
		device->BindTexture(GP::PS, m_GrassTexture.get(), 1);
		device->DrawIndexed(m_TerrainIB->GetNumIndices());

		device->UnbindTexture(GP::VS, 0);
		device->UnbindTexture(GP::PS, 1);
	}

	virtual void ReloadShaders() override
	{
		m_Shader->Reload();
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
public:

	virtual void Init() override
	{
		static const Vec2 WATER_PLANE_POS = Vec2(150.0f, 0.0f);

		m_Shader.reset(new GP::GfxShader("playground/nature/water.hlsl"));
		m_PlaneModel.reset(new GP::ModelTransform());
		m_PlaneModel->SetPosition(Vec3(0.0f, -150.0f, 0.0f));
		m_PlaneModel->SetScale(10000.0f * VEC3_ONE);

		m_DeviceState.reset(new GP::GfxDeviceState());
		m_DeviceState->EnableDepthTest(true);
		m_DeviceState->EnableBackfaceCulling(false);
		m_DeviceState->Compile();
	}

	virtual void Render(GP::GfxDevice* device) override
	{
		GP::DeviceStateScoped _dss(m_DeviceState.get());

		device->BindShader(m_Shader.get());
		device->BindVertexBuffer(GP::GfxDefaults::VB_QUAD);
		device->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(), 0);
		device->BindConstantBuffer(GP::VS, m_PlaneModel->GetBuffer(), 1);
		device->Draw(GP::GfxDefaults::VB_QUAD->GetNumVerts());
	}

	virtual void ReloadShaders() override
	{
		m_Shader->Reload();
	}

private:
	unique_ptr<GP::GfxShader> m_Shader;
	unique_ptr<GP::ModelTransform> m_PlaneModel;
	unique_ptr<GP::GfxDeviceState> m_DeviceState;
};

#ifdef RUN_NATURE_SAMPLE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	GP::Init(hInstance);
	
	GP::Camera playerCamera;
	g_Camera = &playerCamera;

	GP::ShowCursor(false);
	GP::SetDefaultController(g_Camera);
	GP::AddRenderPass(new SkyboxPass());
	GP::AddRenderPass(new WaterPass());
	GP::AddRenderPass(new TerrainPass());
	GP::Run();
	GP::Deinit();
}
#endif // RUN_NATURE_SAMPLE