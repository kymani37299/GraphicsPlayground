#pragma once

#include "Common.h"

namespace GP
{
	namespace Input
	{
		ENGINE_DLL bool IsKeyPressed(unsigned int key);
		ENGINE_DLL bool IsKeyJustPressed(unsigned int key);
		ENGINE_DLL Vec2 GetMousePos();
		ENGINE_DLL Vec2 GetMouseDelta();
	}

	class Controller
	{
	public:
		virtual ~Controller() {}
		virtual void UpdateInput(float dt) {}

		// TODO: Like this
		//virtual void OnMouseMovement(Vec2 delta) {}
		//virtual void OnKeyInput(unsigned int key) {};
	};
}