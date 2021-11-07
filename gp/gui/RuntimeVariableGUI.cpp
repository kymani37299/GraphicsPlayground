#include "RuntimeVariableGUI.h"

#include <imgui.h>

namespace GP
{
	RuntimeVariableGUI* g_RuntimeVariableGUI = new RuntimeVariableGUI();

	void RuntimeFloatSliderGUI::Render()
	{
		ImGui::Text((m_VariableName+" : ").c_str());
		ImGui::SameLine();
		ImGui::SliderFloat("", &m_VariableRef, m_RangeMin, m_RangeMax, "value = %.3f");
	}

	void RuntimeVariableGUI::Render()
	{
		static bool active = true;
		ImGui::Begin("Runtime variables", &active);
		for (RuntimeVariableGUIElement* element : m_RuntimeVariables)
		{
			element->Render();
		}
		ImGui::End();
	}
}