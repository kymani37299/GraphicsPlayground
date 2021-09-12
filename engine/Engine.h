#pragma once

#include <windows.h>

#include "core/Core.h"
#include "GfxCore.h"
#include "RenderPass.h"

namespace GP
{
	ENGINE_DLL void Init(HINSTANCE hInstance);
	ENGINE_DLL void Run();
	ENGINE_DLL void Deinit();

	ENGINE_DLL void AddRenderPass(RenderPass* renderPass);
}