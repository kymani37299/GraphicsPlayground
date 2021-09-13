#include <Engine.h>

#define RUN_NATURE_SAMPLE

GP::Camera* g_Camera = nullptr;

class TerrainPass : public GP::RenderPass
{
	struct TerrainVert
	{
		Vec3 position;
		Vec2 uv;
	};

public:
	TerrainPass()
	{
		unsigned int TERRAIN_SIZE = 10000;
		unsigned int TERRAIN_SIDE_VERTS = 20;
		float TILE_SIZE = (float) TERRAIN_SIZE / TERRAIN_SIDE_VERTS;

		std::vector<TerrainVert> terrainVerts;
		std::vector<unsigned int> terrainIndices;

		terrainVerts.reserve(TERRAIN_SIDE_VERTS * TERRAIN_SIDE_VERTS);
		terrainIndices.reserve((TERRAIN_SIDE_VERTS-1) * (TERRAIN_SIDE_VERTS-1) * 6);
		for (size_t i = 0; i < TERRAIN_SIDE_VERTS; i++)
		{
			for (size_t j = 0; j < TERRAIN_SIDE_VERTS; j++)
			{
				Vec2 pos2D = TILE_SIZE * Vec2(i, j);

				TerrainVert terrainVert;
				terrainVert.position = Vec3(pos2D.x,0.0, pos2D.y);
				terrainVert.uv = (pos2D / (float)TERRAIN_SIZE);

				terrainVerts.push_back(terrainVert);
			}
		}

		for (size_t i = 0; i < TERRAIN_SIDE_VERTS-1; i++)
		{
			for (size_t j = 0; j < TERRAIN_SIDE_VERTS-1; j++)
			{
				terrainIndices.push_back(i + TERRAIN_SIDE_VERTS * j);
				terrainIndices.push_back(i + 1 + TERRAIN_SIDE_VERTS * j );
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
		m_DeviceState->EnableBackfaceCulling(false);
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
	}

	~TerrainPass()
	{
		delete m_TerrainVB;
		delete m_TerrainIB;
		delete m_Shader;
		delete m_DeviceState;
	}

	virtual void Render(GP::GfxDevice* device) override
	{
		GP::DeviceStateScoped dss(m_DeviceState);
		GP::RenderTargetScoped rts(device->GetFinalRT(), device->GetFinalRT());

		device->BindShader(m_Shader);
		device->BindVertexBuffer(m_TerrainVB);
		device->BindIndexBuffer(m_TerrainIB);
		device->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(), 0);
		device->BindTexture(GP::VS, m_HeightMap, 0);
		device->DrawIndexed(m_TerrainIB->GetNumIndices());

		device->UnbindTexture(GP::VS, 0);
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
};

#ifdef RUN_NATURE_SAMPLE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	GP::Init(hInstance);
	
	GP::Camera playerCamera;
	g_Camera = &playerCamera;

	GP::SetDefaultController(g_Camera);
	GP::AddRenderPass(new TerrainPass());
	GP::Run();
	GP::Deinit();
}
#endif // RUN_NATURE_SAMPLE