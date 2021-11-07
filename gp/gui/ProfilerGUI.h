#pragma once

#include "gui/GUI.h"

namespace GP
{
	class ProfilerGUI : public GUIElement
	{
		static constexpr float FPS_UPDATE_INTERVAL = 300.0f; // milliseconds

	public:
		virtual void Update(float dt);
		virtual void Render();

	private:
		int m_FPSSum = 0;
		int m_FPSSampleCount = 0;
		int m_FPS = 0;
		float m_FPSLastUpdate = FPS_UPDATE_INTERVAL;
	};
}