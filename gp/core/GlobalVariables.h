#pragma once

namespace GP
{
	struct GPConfig
	{
		unsigned int WindowWidth = 1024;
		unsigned int WindowHeight = 768;
		unsigned int FPS = 60;
	};

	namespace GlobalVariables
	{
		extern int CURRENT_FPS;
		extern GPConfig GP_CONFIG;
	}
}