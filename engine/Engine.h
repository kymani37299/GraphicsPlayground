#pragma once

#include <windows.h>

#include "Common.h"
#include "core/GfxCore.h"
#include "core/RenderPass.h"
#include "util/ModelLoading.h"

namespace GP
{
	ENGINE_DLL void Init(HINSTANCE hInstance);
	ENGINE_DLL void Run();
	ENGINE_DLL void Deinit();

	ENGINE_DLL void AddRenderPass(RenderPass* renderPass);
}