#include "ProfilerGUI.h"

#include <imgui.h>

#include "Common.h"
#include "core/GlobalVariables.h"

namespace GP
{
	void ProfilerGUI::Update(float dt)
	{
		m_FPSLastUpdate += dt;
		m_FPSSum += GlobalVariables::CURRENT_FPS;
		m_FPSSampleCount++;
		if (m_FPSLastUpdate > FPS_UPDATE_INTERVAL)
		{
			m_FPS = MIN(m_FPSSum / m_FPSSampleCount, GlobalVariables::GP_CONFIG.FPS);
			m_FPSLastUpdate -= FPS_UPDATE_INTERVAL;
			m_FPSSum = 0;
			m_FPSSampleCount = 0;
		}
	}

	void ProfilerGUI::Render()
	{
		static bool active = true;
		ImGui::SetNextWindowSize(ImVec2(0, 0));
		ImGui::Begin("Profiler", &active);
		ImGui::Text("FPS: %d", m_FPS);
		ImGui::End();
	}
}