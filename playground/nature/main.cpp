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
	virtual void Init(GP::GfxDevice*) override { }

	virtual void Render(GP::GfxDevice* device) override 
	{
		RENDER_PASS("Skybox");

		g_SceneRenderer.DrawSkybox(device, g_Camera);
	}

	virtual void ReloadShaders() override {}
};

class TerrainPass : public GP::RenderPass
{
public:
	virtual void Init(GP::GfxDevice*) override { }

	virtual void Render(GP::GfxDevice* device) override
	{
		RENDER_PASS("Terrain");
		g_SceneRenderer.DrawTerrain(device, g_Camera);
	}

	virtual void ReloadShaders() override { }
};

class WaterPass : public GP::RenderPass
{
	const float WATER_REF_RESOLUTION = WINDOW_WIDTH/2.0f;
	const float WATER_HEIGHT = 80.0f;
	const float WATER_HEIGHT_BIAS = 5.0; // Used to remove aliasing when water is slicing terrain

public:

	virtual void Init(GP::GfxDevice* device) override
	{
		RENDER_PASS("WaterPass::Init");

		m_ReflectionCamera.reset(new GP::Camera());

		m_WaterShader.reset(new GP::GfxShader("playground/nature/shaders/water.hlsl"));
		m_PlaneModel.reset(new GP::ModelTransform());
		m_PlaneModel->SetPosition(Vec3(0.0f, WATER_HEIGHT, 0.0f));
		m_PlaneModel->SetScale(10000.0f * VEC3_ONE);

		m_DeviceState.reset(new GP::GfxDeviceState());
		m_DeviceState->EnableDepthTest(true);
		m_DeviceState->EnableBackfaceCulling(false);
		m_DeviceState->Compile();

		m_DuDvMap.reset(new GP::GfxTexture2D("playground/nature/resources/WaterDuDv.png"));

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
		
		{
			RENDER_PASS("Refraction texture");
			GP::RenderTargetScoped _rts(m_WaterRefraction.get(), m_WaterRefraction.get());
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
			device->Clear();

			float clipHeight = WATER_HEIGHT - WATER_HEIGHT_BIAS;
			CBSceneParams params = {};
			params.useClipping = true;
			params.clipPlane = Vec4(0.0f, 1.0f, 0.0f, -clipHeight);

			Vec3 cameraPos = g_Camera->GetPosition();
			cameraPos.y -= 2.0f * (cameraPos.y - WATER_HEIGHT);
			Vec3 cameraRot = g_Camera->GetRotation();
			cameraRot.x = -cameraRot.x;
			m_ReflectionCamera->SetPosition(cameraPos);
			m_ReflectionCamera->SetRotation(cameraRot);

			g_SceneRenderer.DrawSkybox(device, m_ReflectionCamera.get(), params);
			g_SceneRenderer.DrawTerrain(device, m_ReflectionCamera.get(), params);
		}

		{
			RENDER_PASS("Water plane");

			GP::DeviceStateScoped _dss(m_DeviceState.get());

			device->BindShader(m_WaterShader.get());
			device->BindVertexBuffer(GP::GfxDefaults::VB_QUAD);
			device->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(), 0);
			device->BindConstantBuffer(GP::VS, m_PlaneModel->GetBuffer(), 1);
			device->BindConstantBuffer(GP::PS, GP::GetGlobalsBuffer(), 2);
			device->BindTexture(GP::PS, m_WaterReflection.get(), 0);
			device->BindTexture(GP::PS, m_WaterRefraction.get(), 1);
			device->BindTexture2D(GP::PS, m_DuDvMap.get(), 2);
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

	unique_ptr<GP::GfxTexture2D> m_DuDvMap;

	unique_ptr<GP::GfxRenderTarget> m_WaterReflection;
	unique_ptr<GP::GfxRenderTarget> m_WaterRefraction;

	unique_ptr<GP::Camera> m_ReflectionCamera;
};

#ifdef RUN_NATURE_SAMPLE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	GP::Init(hInstance);
	
	GP::Camera playerCamera;
	g_Camera = &playerCamera;
	g_Camera->SetPosition({ 0.0,100.0,0.0 });

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