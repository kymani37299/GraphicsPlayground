#pragma once

#include <Engine.h>

#include "PlaygroundSample.h"

namespace SponzaSample
{
	GP::Camera* g_Camera = nullptr;

	class SponzaRenderer : public GP::RenderPass
	{
	public:
		virtual void Init(GP::GfxDevice* device) override
		{
			m_Scene = GP::SceneLoading::LoadScene("playground/sponza/resources/sponza/sponza.gltf");
			m_DeviceState.EnableDepthTest(true);
			m_DeviceState.Compile();

			m_Shader = new GP::GfxShader("playground/sponza/shaders/opaque.hlsl");
		}

		virtual void Render(GP::GfxDevice* device) override
		{
			GP::DeviceStateScoped _dds(&m_DeviceState);
			device->BindShader(m_Shader);
			for (const GP::SceneObject* sceneObject : m_Scene->GetObjects())
			{
				const GP::Mesh* mesh = sceneObject->GetMesh();

				ID3D11Buffer* buffers[] = { 
					mesh->GetPositionBuffer()->GetBufferResource()->GetBuffer(), 
					mesh->GetUVBuffer()->GetBufferResource()->GetBuffer(), 
					mesh->GetNormalBuffer()->GetBufferResource()->GetBuffer(),
					mesh->GetTangentBuffer()->GetBufferResource()->GetBuffer() };

				unsigned int strides[] = {
					mesh->GetPositionBuffer()->GetStride(),
					mesh->GetUVBuffer()->GetStride(),
					mesh->GetNormalBuffer()->GetStride(),
					mesh->GetTangentBuffer()->GetStride() };

				unsigned int offsets[] = {
					mesh->GetPositionBuffer()->GetOffset(),
					mesh->GetUVBuffer()->GetOffset(),
					mesh->GetNormalBuffer()->GetOffset(),
					mesh->GetTangentBuffer()->GetOffset()
				};

				device->BindVertexBuffer(4, buffers, strides, offsets);
				device->BindIndexBuffer(mesh->GetIndexBuffer());
				device->BindConstantBuffer(GP::VS, g_Camera->GetBuffer(), 0);
				device->BindTexture2D(GP::PS, sceneObject->GetMaterial()->GetDiffuseTexture(), 0);
				device->DrawIndexed(mesh->GetPositionBuffer()->GetNumVerts());
			}
		}

		virtual void ReloadShaders() override
		{
			m_Shader->Reload();
		}

	private:
		GP::Scene* m_Scene;
		GP::GfxDeviceState m_DeviceState;
		GP::GfxShader* m_Shader;
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