#pragma once

#include <windows.h>

#include "Common.h"
#include "gfx/GfxDevice.h"
#include "gfx/GfxTransformations.h"
#include "gfx/GfxBuffers.h"
#include "gfx/GfxTexture.h"
#include "gfx/GfxShader.h"
#include "gfx/ScopedOperations.h"

#include "core/Controller.h"
#include "core/RenderPass.h"

#include "defaults/DefaultController.h"
#include "defaults/DefaultSceneRenderPass.h"
#include "defaults/DefaultSkyboxRenderPass.h"
#include "defaults/UtilRenderPasses.h"

#include "debug/Logger.h"
#include "debug/RuntimeVariable.h"

// SCENE_SUPPORT
#include "scene/Scene.h"

namespace GP
{
	struct CBEngineGlobals;

	// Game state
	GP_DLL void Init(HINSTANCE hInstance, unsigned int windowWidth, unsigned int windowHeight, const std::string& windowTitle = "", unsigned int fps = 60);
	GP_DLL void Run();
	GP_DLL void Reset();
	GP_DLL void Deinit();

	// Initialization
	GP_DLL void SetController(Controller* controller);
	GP_DLL void AddRenderPass(RenderPass* renderPass);

	// Runtime
	GP_DLL void ShowCursor(bool show);
	GP_DLL void Shutdown();
	GP_DLL void ReloadShaders();
	GP_DLL GfxConstantBuffer<CBEngineGlobals>* GetGlobalsBuffer();
}