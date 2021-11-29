#pragma once

#include "core/RenderPass.h"
#include "scene/Scene.h"
#include "gfx/GfxDevice.h"
#include "gfx/GfxShader.h"

namespace GP
{
	class Camera;

	// Default renderer for Scene. Phong lighting
	class DefaultSceneRenderPass : public RenderPass
	{
	public:
		DefaultSceneRenderPass(Camera* camera) :
			m_Camera(camera) {}

		GP_DLL ~DefaultSceneRenderPass();

		inline void Load(const std::string& path, Vec3 position = VEC3_ZERO, Vec3 scale = VEC3_ONE, Vec3 rotation = VEC3_ZERO)
		{
			m_Scene.Load(path, position, scale, rotation);
		}

		GP_DLL virtual void Init(GfxContext* context) override;
		GP_DLL virtual void Render(GfxContext* context) override;

		inline virtual void ReloadShaders() override
		{
			m_ShaderOpaque->Reload();
			m_ShaderTransparent->Reload();
		}

	private:
		Scene m_Scene;
		Camera* m_Camera = nullptr;
		GfxShader* m_ShaderOpaque = nullptr;
		GfxShader* m_ShaderTransparent = nullptr;
		GfxSampler* m_DiffuseSampler = nullptr;
	};
}