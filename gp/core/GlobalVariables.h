#pragma once

namespace GP
{
	struct GPConfig
	{
		unsigned int WindowWidth = 1024;
		unsigned int WindowHeight = 768;
		bool WindowSizeDirty = false;
		unsigned int FPS = 60;
		bool VSYNC = false;
	};

	namespace GlobalVariables
	{
		extern int CURRENT_FPS;
		extern GPConfig GP_CONFIG;
	}
}