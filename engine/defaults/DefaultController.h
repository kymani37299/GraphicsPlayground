#pragma once

#include "core/Controller.h"

namespace GP
{
	class Camera;

	class DefaultController : public Controller
	{
	public:
		DefaultController(Camera& camera) : m_Camera(camera) {}
		virtual void UpdateInput(float dt);

	private:
		Camera& m_Camera;
	};
}