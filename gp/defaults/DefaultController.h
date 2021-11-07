#pragma once

#include "core/Controller.h"

namespace GP
{
	class Camera;

	class DefaultController : public Controller
	{
	public:
		DefaultController(Camera& camera) : m_Camera(camera) {}
		GP_DLL virtual void UpdateInput(float dt);

	private:
		void UpdateGameInput(float dt);

	private:
		Camera& m_Camera;
	};
}