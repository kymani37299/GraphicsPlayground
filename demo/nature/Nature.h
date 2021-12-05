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
		// TODO: OnWindowResized , and get those values right
		static constexpr float WATER_REF_RESOLUTION = 1024 / 2.0f; // 1024 == Window Width
		static constexpr float WATER_HEIGHT_BIAS = 5.0; // Used to remove aliasing when water is slicing terrain

	public:

		virtual void Init(GP::GfxContext* context) override
		{
			m_PlaneModel.SetScale(10000.0f * VEC3_ONE);
		}

		virtual void Render(GP::GfxContext* context) override
		{
			if (!m_EnableWaterVariable.GetValue()) return;

			GP_SCOPED_PROFILE("Water");

			m_PlaneModel.SetPosition(Vec3(0.0f, m_WaterlevelVariable.GetValue(), 0.0f));

			{
				GP_SCOPED_PROFILE("Refraction texture");
				GP_SCOPED_RT(context, &m_WaterRefraction, &m_WaterRefraction);

				context->Clear();

				float clipHeight = m_WaterlevelVariable.GetValue() + WATER_HEIGHT_BIAS;
				CBSceneParams params = {};
				params.useClipping = true;
				params.clipPlane = Vec4(0.0f, -1.0f, 0.0f, clipHeight);

				g_SceneRenderer.DrawTerrain(context, g_Camera, params);
			}

			{
				GP_SCOPED_PROFILE("Reflection texture");
				GP_SCOPED_RT(context, &m_WaterReflection, &m_WaterReflection);

				context->Clear();

				float clipHeight = m_WaterlevelVariable.GetValue() - WATER_HEIGHT_BIAS;
				CBSceneParams params = {};
				params.useClipping = true;
				params.clipPlane = Vec4(0.0f, 1.0f, 0.0f, -clipHeight);

				Vec3 cameraPos = g_Camera->GetPosition();
				cameraPos.y -= 2.0f * (cameraPos.y - m_WaterlevelVariable.GetValue());
				Vec3 cameraRot = g_Camera->GetRotation();
				cameraRot.x = -cameraRot.x;
				m_ReflectionCamera.SetPosition(cameraPos);
				m_ReflectionCamera.SetRotation(cameraRot);

				g_SceneRenderer.DrawSkybox(context, &m_ReflectionCamera, params);
				g_SceneRenderer.DrawTerrain(context, &m_ReflectionCamera, params);
			}

			{
				GP_SCOPED_PROFILE("Water plane");

				context->BindShader(&m_WaterShader);
				context->BindVertexBuffer(GP::GfxDefaults::VB_QUAD);
				context->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(context), 0);
				context->BindConstantBuffer(GP::VS, m_PlaneModel.GetBuffer(context), 1);
				context->BindConstantBuffer(GP::PS, GP::GetGlobalsBuffer(), 2);
				context->BindTexture2D(GP::PS, &m_WaterReflectionTexture, 0);
				context->BindTexture2D(GP::PS, &m_WaterRefractionTexture, 1);
				context->BindTexture2D(GP::PS, &m_DuDvMap, 2);
				context->Draw(GP::GfxDefaults::VB_QUAD->GetNumVerts());

				context->UnbindTexture(GP::PS, 0);
				context->UnbindTexture(GP::PS, 1);
			}

		}

		virtual void ReloadShaders() override
		{
			m_WaterShader.Reload();
		}

	private:

		static constexpr unsigned int RT_WIDTH = (unsigned int)(WATER_REF_RESOLUTION * (1024.0f / 768.0f)); // (1024.0f / 768.0f) == ASPECT_RATIO
		static constexpr unsigned int RT_HEIGHT = (unsigned int)WATER_REF_RESOLUTION;

		GP::RuntimeFloat m_WaterlevelVariable{ "Water level", 80.0f, -100.0f, 100.0f };
		GP::RuntimeBool m_EnableWaterVariable{ "Enable water", true };

		GP::GfxShader m_WaterShader{ "demo/nature/shaders/water.hlsl" };
		GP::ModelTransform m_PlaneModel;

		GP::GfxTexture2D m_DuDvMap{ "demo/nature/resources/WaterDuDv.png" };

		GP::GfxRenderTarget m_WaterReflection{ RT_WIDTH, RT_HEIGHT, 1, true };
		GP::GfxRenderTarget m_WaterRefraction{ RT_WIDTH, RT_HEIGHT, 1, true };

		GP::GfxTexture2D m_WaterReflectionTexture{ m_WaterReflection.GetResource(0) };
		GP::GfxTexture2D m_WaterRefractionTexture{ m_WaterRefraction.GetResource(0) };

		GP::Camera m_ReflectionCamera;
	};

	class CloudsPass : public GP::RenderPass
	{
	public:

		virtual void Init(GP::GfxContext* context) override
		{
			GP::GfxShader generatePelinShader{ "demo/nature/shaders/generate_3d_perlin.hlsl" };

			{
				GP_SCOPED_PROFILE("Generate 3D perlin");
				context->BindRWTexture3D(GP::CS, &m_Perlin3D, 0);
				context->BindShader(&generatePelinShader);
				context->Dispatch(128, 128, 128);
			}
		}

		virtual void Render(GP::GfxContext* context) override {}

	private:
		GP::GfxRWTexture3D m_Perlin3D{ 128,128,128 };
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
