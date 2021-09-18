#include <Engine.h>

#include <memory>

#include "SceneRenderer.h"

using namespace std;

#define RUN_NATURE_SAMPLE

GP::Camera* g_Camera = nullptr;
SceneRenderer g_SceneRenderer;

class SkyboxPass : public GP::RenderPass
{
public:

	virtual void Init() override 
	{
		m_SkyboxDeviceState.reset(new GP::GfxDeviceState());
		m_SkyboxDeviceState->EnableBackfaceCulling(false);
		m_SkyboxDeviceState->Compile();
	}

	virtual void Render(GP::GfxDevice* device) override 
	{
		RENDER_PASS("Skybox");
		GP::DeviceStateScoped _dss(m_SkyboxDeviceState.get());
		g_SceneRenderer.DrawSkybox(device, g_Camera);
	}

	virtual void ReloadShaders() override {}

private:
	unique_ptr<GP::GfxDeviceState> m_SkyboxDeviceState;
};

class TerrainPass : public GP::RenderPass
{
public:

	virtual void Init() override
	{
		m_TerrainDeviceState.reset(new GP::GfxDeviceState());
		m_TerrainDeviceState->EnableDepthTest(true);
		m_TerrainDeviceState->Compile();
	}

	virtual void Render(GP::GfxDevice* device) override
	{
		RENDER_PASS("Terrain");

		GP::DeviceStateScoped dss(m_TerrainDeviceState.get());
		g_SceneRenderer.DrawTerrain(device, g_Camera);
	}

	virtual void ReloadShaders() override { }

private:
	std::unique_ptr<GP::GfxDeviceState> m_TerrainDeviceState;
};

class WaterPass : public GP::RenderPass
{
	const float WATER_REF_RESOLUTION = WINDOW_WIDTH/2.0f;
	const float WATER_HEIGHT = -50.0f;
	const float WATER_HEIGHT_BIAS = 5.0; // Used to remove aliasing when water is slicing terrain

public:

	virtual void Init() override
	{
		RENDER_PASS("WaterPass::Init");

		m_WaterShader.reset(new GP::GfxShader("playground/nature/shaders/water.hlsl"));
		m_PlaneModel.reset(new GP::ModelTransform());
		m_PlaneModel->SetPosition(Vec3(0.0f, WATER_HEIGHT, 0.0f));
		m_PlaneModel->SetScale(10000.0f * VEC3_ONE);

		m_DeviceState.reset(new GP::GfxDeviceState());
		m_DeviceState->EnableDepthTest(true);
		m_DeviceState->EnableBackfaceCulling(false);
		m_DeviceState->Compile();

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
		
		{
			RENDER_PASS("Refraction texture");
			GP::RenderTargetScoped _rts(m_WaterRefraction.get(), m_WaterRefraction.get());
			// TODO: Add device state
			device->Clear();

			float clipHeight = WATER_HEIGHT + WATER_HEIGHT_BIAS;
			CBSceneParams params = {};
			params.useClipping = true;
			params.clipPlane = Vec4(0.0f, -1.0f, 0.0f, clipHeight);

			g_SceneRenderer.DrawTerrain(device, g_Camera, params);
		}

		{
			RENDER_PASS("Reflection texture");

			GP::RenderTargetScoped _rts(m_WaterReflection.get(), m_WaterReflection.get());
			// TODO: Add device state
			device->Clear();

			float clipHeight = WATER_HEIGHT + WATER_HEIGHT_BIAS;
			CBSceneParams params = {};
			params.useClipping = true;
			params.clipPlane = Vec4(0.0f, 1.0f, 0.0f, -clipHeight);

			// Setup camera under the water - TODO: Maybe do this in some other camera buffer#
			Vec3 camPos = g_Camera->GetPosition();
			float playerWaterHeight = camPos.y - WATER_HEIGHT;
			camPos.y -= 2.0f * playerWaterHeight;
			Vec3 camRot = g_Camera->GetRotation();
			camRot.x = -camRot.x;
			g_Camera->SetPosition(camPos);
			g_Camera->SetRotation(camRot);
			///

			g_SceneRenderer.DrawSkybox(device, g_Camera, params);
			g_SceneRenderer.DrawTerrain(device, g_Camera, params);

			// Revert camera changes
			camPos.y += 2.0f * playerWaterHeight;
			camRot.x = -camRot.x;
			g_Camera->SetPosition(camPos);
			g_Camera->SetRotation(camRot);
			///
		}

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
		m_WaterShader->Reload();
	}

private:
	unique_ptr<GP::GfxShader> m_WaterShader;
	unique_ptr<GP::ModelTransform> m_PlaneModel;
	unique_ptr<GP::GfxDeviceState> m_DeviceState;

	unique_ptr<GP::GfxRenderTarget> m_WaterReflection;
	unique_ptr<GP::GfxRenderTarget> m_WaterRefraction;
};

#ifdef RUN_NATURE_SAMPLE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	GP::Init(hInstance);
	
	GP::Camera playerCamera;
	g_Camera = &playerCamera;
	g_Camera->SetPosition({ -30.0,50.0,0.0 });

	GP::ShowCursor(false);
	GP::SetDefaultController(g_Camera);
	GP::AddRenderPass(new ScenePass(&g_SceneRenderer));
	GP::AddRenderPass(new SkyboxPass());
	GP::AddRenderPass(new TerrainPass());
	GP::AddRenderPass(new WaterPass());
	GP::Run();
	GP::Deinit();
}
#endif // RUN_NATURE_SAMPLE