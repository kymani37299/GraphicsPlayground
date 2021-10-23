#pragma once

#include <windows.h>

#include "Common.h"
#include "gfx/GfxCore.h"
#include "gfx/GfxTransformations.h"
#include "gfx/GfxBuffers.h"
#include "gfx/GfxTexture.h"

#include "core/Controller.h"
#include "core/RenderPass.h"

// SCENE_SUPPORT
#include "scene/SceneLoading.h"
#include "scene/Scene.h"

namespace GP
{
	struct CBEngineGlobals;

	// Game state
	ENGINE_DLL void Init(HINSTANCE hInstance);
	ENGINE_DLL void Run();
	ENGINE_DLL void Deinit();

	// Initialization
	ENGINE_DLL void SetDefaultController(Camera* camera);
	ENGINE_DLL void SetController(Controller* controller);
	ENGINE_DLL void AddRenderPass(RenderPass* renderPass);

	// Runtime
	ENGINE_DLL void ShowCursor(bool show);
	ENGINE_DLL void Shutdown();
	ENGINE_DLL void ReloadShaders();
	ENGINE_DLL GfxConstantBuffer<CBEngineGlobals>* GetGlobalsBuffer();
}