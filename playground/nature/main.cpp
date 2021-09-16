#include <Engine.h>

#define RUN_NATURE_SAMPLE

GP::Camera* g_Camera = nullptr;

class SkyboxPass : public GP::RenderPass
{
public:
	virtual ~SkyboxPass() 
	{
		delete m_SkyboxTexture;
		delete m_DeviceState;
		delete m_Shader;
	}

	virtual void Init() override 
	{
		m_DeviceState = new GP::GfxDeviceState();
		m_DeviceState->EnableBackfaceCulling(false);
		m_DeviceState->Compile();

		GP::ShaderDesc shaderDesc = {};
		shaderDesc.path = "playground/nature/skybox.hlsl";
		shaderDesc.inputs.resize(1);
		shaderDesc.inputs[0] = { GP::ShaderInputFormat::Float3 , "POS" };
		m_Shader = new GP::GfxShader(shaderDesc);

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

	virtual void Render(GP::GfxDevice* device) override 
	{
		RENDER_PASS("Skybox");

		GP::DeviceStateScoped _dss(m_DeviceState);
		device->BindShader(m_Shader);
		device->BindVertexBuffer(GP::GfxDefaults::VB_CUBE);
		device->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(), 0);
		device->BindTexture(GP::PS, m_SkyboxTexture, 0);
		device->Draw(GP::GfxDefaults::VB_CUBE->GetNumVerts());
		device->UnbindTexture(GP::PS, 0);
	}

	virtual void ReloadShaders() override 
	{
		m_Shader->Reload();
	}

private:
	GP::GfxDeviceState* m_DeviceState;
	GP::GfxShader* m_Shader;
	GP::GfxTexture* m_SkyboxTexture;
};

class TerrainPass : public GP::RenderPass
{
	struct TerrainVert
	{
		Vec3 position;
		Vec2 uv;
	};

public:

	~TerrainPass()
	{
		delete m_TerrainVB;
		delete m_TerrainIB;
		delete m_Shader;
		delete m_DeviceState;
		delete m_HeightMap;
		delete m_GrassTexture;
	}

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
		m_TerrainVB = new GP::GfxVertexBuffer(vertexData);
		m_TerrainIB = new GP::GfxIndexBuffer(terrainIndices.data(), terrainIndices.size());

		m_DeviceState = new GP::GfxDeviceState();
		m_DeviceState->EnableDepthTest(true);
		m_DeviceState->Compile();

		GP::ShaderDesc shaderDesc = {};
		shaderDesc.path = "playground/nature/terrain.hlsl";
		shaderDesc.inputs.resize(2);
		shaderDesc.inputs[0] = { GP::ShaderInputFormat::Float3 , "POS" };
		shaderDesc.inputs[1] = { GP::ShaderInputFormat::Float2 , "TEXCOORD" };
		m_Shader = new GP::GfxShader(shaderDesc);

		GP::SceneLoading::TextureData* heightmapData = GP::SceneLoading::LoadTexture("playground/nature/resources/PerlinNoise.png");
		GP::TextureDesc heightmapDesc = {};
		heightmapDesc.height = heightmapData->height;
		heightmapDesc.width = heightmapData->width;
		heightmapDesc.texData.push_back(heightmapData->pData);
		heightmapDesc.type = GP::TextureType::Texture2D;
		heightmapDesc.format = GP::TextureFormat::RGBA8_UNORM;
		m_HeightMap = new GP::GfxTexture(heightmapDesc);
		GP::SceneLoading::FreeTexture(heightmapData);

		GP::SceneLoading::TextureData* grassData = GP::SceneLoading::LoadTexture("playground/nature/resources/grass.png");
		GP::TextureDesc grassDesc = {};
		grassDesc.height = grassData->height;
		grassDesc.width = grassData->width;
		grassDesc.texData.push_back(grassData->pData);
		grassDesc.type = GP::TextureType::Texture2D;
		grassDesc.format = GP::TextureFormat::RGBA8_UNORM;
		m_GrassTexture = new GP::GfxTexture(grassDesc);
		GP::SceneLoading::FreeTexture(grassData);
	}

	virtual void Render(GP::GfxDevice* device) override
	{
		RENDER_PASS("Terrain");

		GP::DeviceStateScoped dss(m_DeviceState);
		GP::RenderTargetScoped rts(device->GetFinalRT(), device->GetFinalRT());

		device->BindShader(m_Shader);
		device->BindVertexBuffer(m_TerrainVB);
		device->BindIndexBuffer(m_TerrainIB);
		device->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(), 0);
		device->BindTexture(GP::VS, m_HeightMap, 0);
		device->BindTexture(GP::PS, m_GrassTexture, 1);
		device->DrawIndexed(m_TerrainIB->GetNumIndices());

		device->UnbindTexture(GP::VS, 0);
		device->UnbindTexture(GP::PS, 1);
	}

	virtual void ReloadShaders() override
	{
		m_Shader->Reload();
	}

private:
	GP::GfxVertexBuffer* m_TerrainVB;
	GP::GfxIndexBuffer* m_TerrainIB;

	GP::GfxShader* m_Shader;
	GP::GfxDeviceState* m_DeviceState;

	GP::GfxTexture* m_HeightMap;
	GP::GfxTexture* m_GrassTexture;
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
	GP::AddRenderPass(new TerrainPass());
	GP::Run();
	GP::Deinit();
}
#endif // RUN_NATURE_SAMPLE