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

		inline void Load(const std::string& path, Vec3 position = VEC3_ZERO, Vec3 scale = VEC3_ONE, Vec3 rotation = VEC3_ZERO)
		{
			m_Scene.Load(path, position, scale, rotation);
		}

		ENGINE_DLL virtual void Init(GfxDevice* device) override;
		ENGINE_DLL virtual void Render(GfxDevice* device) override;

		inline virtual void ReloadShaders() override
		{
			m_Shader->Reload();
		}

	private:
		Scene m_Scene;
		Camera* m_Camera = nullptr;
		GfxDeviceState m_DeviceStateOpaque;
		GfxDeviceState m_DeviceStateTransparent;
		GfxShader* m_Shader = nullptr;
		GfxSampler* m_DiffuseSampler = nullptr;
	};
}