#pragma once

#include "core/RenderPass.h"
#include "scene/Scene.h"
#include "gfx/GfxCore.h"

namespace GP
{
	class Camera;

	// Default renderer for Scene. Phong lighting
	class DefaultSceneRenderPass : public RenderPass
	{
	public:
		DefaultSceneRenderPass(Camera* camera) :
			m_Camera(camera) {}

		ENGINE_DLL ~DefaultSceneRenderPass();

		inline void Load(const std::string& path)
		{
			m_Scene.Load(path);
		}

		ENGINE_DLL virtual void Init(GfxDevice* device) override;
		ENGINE_DLL virtual void Render(GfxDevice* device) override;

	private:
		Scene m_Scene;
		Camera* m_Camera = nullptr;
		GfxDeviceState m_DeviceState;
		GfxShader* m_Shader = nullptr;
		GfxSampler* m_DiffuseSampler = nullptr;
	};
}