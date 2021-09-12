#pragma once

#include <windows.h>

#include "core/Core.h"

class Scene;

namespace GP
{
	ENGINE_DLL void Init(HINSTANCE hInstance);
	ENGINE_DLL void Run();
	ENGINE_DLL void Deinit();
}