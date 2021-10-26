#pragma once

#include "Common.h"

#ifdef SCENE_SUPPORT

#include <string>

namespace GP
{
	class Scene;

	namespace SceneLoading
	{
		ENGINE_DLL Scene* LoadScene(const std::string& path);
	}
}

#endif // SCENE_SUPPORT
