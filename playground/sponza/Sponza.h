#pragma once

#include <GP.h>

#include "PlaygroundSample.h"

namespace SponzaSample
{
	GP::Camera* g_Camera = nullptr;

	class SponzaRenderer : public GP::RenderPass
	{
	public:
		virtual ~SponzaRenderer()
		{
			delete m_Shader;
			delete m_DiffuseSampler;
		}

		virtual void Init(GP::GfxDevice* device) override
		{
			m_Scene.Load("playground/sponza/resources/sponza/sponza.gltf");
			m_DeviceState.EnableDepthTest(true);
			m_DeviceState.EnableBackfaceCulling(false);
			m_DeviceState.Compile();

			m_Shader = new GP::GfxShader("playground/sponza/shaders/opaque.hlsl");
			m_DiffuseSampler = new GP::GfxSampler(GP::SamplerFilter::Anisotropic, GP::SamplerMode::Wrap);
		}

		virtual void Render(GP::GfxDevice* device) override
		{
			RENDER_PASS("Sponza");

			GP::DeviceStateScoped _dds(&m_DeviceState);
			device->BindShader(m_Shader);
			device->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(), 0);
			device->BindSampler(GP::PS, m_DiffuseSampler, 0);
			m_Scene.ForEverySceneObject([device](const GP::SceneObject* sceneObject){
				const GP::Mesh* mesh = sceneObject->GetMesh();
				device->BindVertexBufferSlot(mesh->GetPositionBuffer(), 0);
 				device->BindVertexBufferSlot(mesh->GetUVBuffer(), 1);
				device->BindVertexBufferSlot(mesh->GetNormalBuffer(), 2);
				device->BindVertexBufferSlot(mesh->GetTangentBuffer(), 3);
				device->BindIndexBuffer(mesh->GetIndexBuffer());
				device->BindTexture2D(GP::PS, sceneObject->GetMaterial()->GetDiffuseTexture(), 0);
				device->DrawIndexed(mesh->GetIndexBuffer()->GetNumIndices());
			});
		}

		virtual void ReloadShaders() override
		{
			m_Shader->Reload();
		}

	private:
		GP::Scene m_Scene;
		GP::GfxDeviceState m_DeviceState;
		GP::GfxShader* m_Shader;
		GP::GfxSampler* m_DiffuseSampler;
	};

	class SponzaSample : public PlaygroundSample
	{
	public:
		void SetupRenderer() override
		{
			g_Camera = &m_PlayerCamera;
			g_Camera->SetPosition({ 0.0,100.0,0.0 });

			GP::ShowCursor(false);
			GP::SetDefaultController(g_Camera);
			GP::AddRenderPass(new SponzaRenderer());
		}

	private:
		GP::Camera m_PlayerCamera;
	};
}