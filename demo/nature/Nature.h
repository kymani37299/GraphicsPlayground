#pragma once

#include "DemoSample.h"

#include <GP.h>

#include <memory>

#include "SceneRenderer.h"

using namespace std;

extern GP::Camera* g_Camera;

namespace NatureSample
{
	SceneRenderer g_SceneRenderer;

	class SkyboxPass : public GP::RenderPass
	{
	public:
		virtual void Init(GP::GfxContext*) override { }

		virtual void Render(GP::GfxContext* context) override
		{
			GP_SCOPED_PROFILE("Skybox");

			g_SceneRenderer.DrawSkybox(context, g_Camera);
		}

		virtual void ReloadShaders() override {}
	};

	class TerrainPass : public GP::RenderPass
	{
	public:
		virtual void Init(GP::GfxContext*) override { }

		virtual void Render(GP::GfxContext* context) override
		{
			GP_SCOPED_PROFILE("Terrain");
			g_SceneRenderer.DrawTerrain(context, g_Camera);
		}

		virtual void ReloadShaders() override { }
	};

	class WaterPass : public GP::RenderPass
	{
		const float WATER_REF_RESOLUTION = WINDOW_WIDTH / 2.0f;
		const float WATER_HEIGHT_BIAS = 5.0; // Used to remove aliasing when water is slicing terrain

	public:

		virtual void Init(GP::GfxContext* context) override
		{
			GP_SCOPED_PROFILE("WaterPass::Init");

			m_ReflectionCamera.reset(new GP::Camera());

			m_WaterShader.reset(new GP::GfxShader("demo/nature/shaders/water.hlsl"));
			m_PlaneModel.reset(new GP::ModelTransform());
			m_PlaneModel->SetScale(10000.0f * VEC3_ONE);

			m_DeviceState.reset(new GP::GfxDeviceState());
			m_DeviceState->EnableDepthTest(true);
			m_DeviceState->EnableBackfaceCulling(false);
			m_DeviceState->Compile();

			m_DuDvMap.reset(new GP::GfxTexture2D("demo/nature/resources/WaterDuDv.png"));

			const unsigned int rtWidth = (unsigned int) (WATER_REF_RESOLUTION * ASPECT_RATIO);
			const unsigned int rtHeight = (unsigned int) WATER_REF_RESOLUTION;
			m_WaterRefraction.reset(new GP::GfxRenderTarget(rtWidth, rtHeight, 1, true));
			m_WaterReflection.reset(new GP::GfxRenderTarget(rtWidth, rtHeight, 1, true));

			m_WaterRefractionTexture.reset(new GP::GfxTexture2D(m_WaterRefraction->GetResource()));
			m_WaterReflectionTexture.reset(new GP::GfxTexture2D(m_WaterReflection->GetResource()));
		}

		virtual void Render(GP::GfxContext* context) override
		{
			if (!m_EnableWaterVariable.GetValue()) return;

			GP_SCOPED_PROFILE("Water");

			m_PlaneModel->SetPosition(Vec3(0.0f, m_WaterlevelVariable.GetValue(), 0.0f));

			{
				GP_SCOPED_PROFILE("Refraction texture");
				GP_SCOPED_RT(m_WaterRefraction.get(), m_WaterReflection.get());

				context->Clear();

				float clipHeight = m_WaterlevelVariable.GetValue() + WATER_HEIGHT_BIAS;
				CBSceneParams params = {};
				params.useClipping = true;
				params.clipPlane = Vec4(0.0f, -1.0f, 0.0f, clipHeight);

				g_SceneRenderer.DrawTerrain(context, g_Camera, params);
			}

			{
				GP_SCOPED_PROFILE("Reflection texture");
				GP_SCOPED_RT(m_WaterReflection.get(), m_WaterReflection.get());

				context->Clear();

				float clipHeight = m_WaterlevelVariable.GetValue() - WATER_HEIGHT_BIAS;
				CBSceneParams params = {};
				params.useClipping = true;
				params.clipPlane = Vec4(0.0f, 1.0f, 0.0f, -clipHeight);

				Vec3 cameraPos = g_Camera->GetPosition();
				cameraPos.y -= 2.0f * (cameraPos.y - m_WaterlevelVariable.GetValue());
				Vec3 cameraRot = g_Camera->GetRotation();
				cameraRot.x = -cameraRot.x;
				m_ReflectionCamera->SetPosition(cameraPos);
				m_ReflectionCamera->SetRotation(cameraRot);

				g_SceneRenderer.DrawSkybox(context, m_ReflectionCamera.get(), params);
				g_SceneRenderer.DrawTerrain(context, m_ReflectionCamera.get(), params);
			}

			{
				GP_SCOPED_PROFILE("Water plane");
				GP_SCOPED_STATE(m_DeviceState.get());

				context->BindShader(m_WaterShader.get());
				context->BindVertexBuffer(GP::GfxDefaults::VB_QUAD);
				context->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(), 0);
				context->BindConstantBuffer(GP::VS, m_PlaneModel->GetBuffer(), 1);
				context->BindConstantBuffer(GP::PS, GP::GetGlobalsBuffer(), 2);
				context->BindTexture2D(GP::PS, m_WaterReflectionTexture.get(), 0);
				context->BindTexture2D(GP::PS, m_WaterRefractionTexture.get(), 1);
				context->BindTexture2D(GP::PS, m_DuDvMap.get(), 2);
				context->Draw(GP::GfxDefaults::VB_QUAD->GetNumVerts());

				context->UnbindTexture(GP::PS, 0);
				context->UnbindTexture(GP::PS, 1);
			}

		}

		virtual void ReloadShaders() override
		{
			m_WaterShader->Reload();
		}

	private:

		GP::RuntimeFloat m_WaterlevelVariable{ "Water level", 80.0f, -100.0f, 100.0f };
		GP::RuntimeBool m_EnableWaterVariable{ "Enable water", true };

		unique_ptr<GP::GfxShader> m_WaterShader;
		unique_ptr<GP::ModelTransform> m_PlaneModel;
		unique_ptr<GP::GfxDeviceState> m_DeviceState;

		unique_ptr<GP::GfxTexture2D> m_DuDvMap;

		unique_ptr<GP::GfxRenderTarget> m_WaterReflection;
		unique_ptr<GP::GfxRenderTarget> m_WaterRefraction;

		unique_ptr<GP::GfxTexture2D> m_WaterReflectionTexture;
		unique_ptr<GP::GfxTexture2D> m_WaterRefractionTexture;

		unique_ptr<GP::Camera> m_ReflectionCamera;
	};

	class CloudsPass : public GP::RenderPass
	{
	public:

		virtual void Init(GP::GfxContext* context) override
		{
			GP::GfxShader generatePelinShader{ "demo/nature/shaders/generate_3d_perlin.hlsl" };
			GP::GfxRWTexture3D perlin3D{ 128,128,128,1, GP::TRCF_BindSRV };
			m_Perlin3D.reset(new GP::GfxTexture3D(perlin3D));

			{
				GP_SCOPED_PROFILE("Generate 3D perlin");
				context->BindRWTexture3D(GP::CS, &perlin3D, 0);
				context->BindShader(&generatePelinShader);
				context->Dispatch(128, 128, 128);
			}
		}

		virtual void Render(GP::GfxContext* context) override {}

	private:
		unique_ptr<GP::GfxTexture3D> m_Perlin3D;
	};

	class NatureSample : public DemoSample
	{
	public:
		void SetupRenderer() override
		{
			GP::AddRenderPass(new ScenePass(&g_SceneRenderer));
			GP::AddRenderPass(new SkyboxPass());
			GP::AddRenderPass(new TerrainPass());
			GP::AddRenderPass(new WaterPass());
			GP::AddRenderPass(new CloudsPass());
		}
	};
}
