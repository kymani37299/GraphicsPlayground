#pragma once

#include <windows.h>

#include "Common.h"
#include "core/GfxCore.h"
#include "core/GfxTransformations.h"

#include "core/Controller.h"
#include "core/RenderPass.h"

// SCENE_SUPPORT
#include "scene/SceneLoading.h"
#include "scene/Scene.h"

namespace GP
{
	// Game state
	ENGINE_DLL void Init(HINSTANCE hInstance);
	ENGINE_DLL void Run();
	ENGINE_DLL void Deinit();

	// Initialization
	ENGINE_DLL void SetController(Controller* controller);
	ENGINE_DLL void AddRenderPass(RenderPass* renderPass);

	// Runtime
	ENGINE_DLL void Shutdown();
	ENGINE_DLL void ReloadShaders();
}