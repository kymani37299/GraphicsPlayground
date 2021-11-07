#pragma once

#include "Common.h"

namespace GP
{
	namespace Input
	{
		GP_DLL bool IsKeyPressed(unsigned int key);
		GP_DLL bool IsKeyJustPressed(unsigned int key);
		GP_DLL Vec2 GetMousePos();
		GP_DLL Vec2 GetMouseDelta();
	}

	class Controller
	{
	public:
		virtual ~Controller() {}
		virtual void UpdateInput(float dt) {}
	};
}