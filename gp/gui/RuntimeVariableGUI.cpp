#include "RuntimeVariableGUI.h"

#include <imgui.h>

namespace GP
{
	RuntimeVariableGUI* g_RuntimeVariableGUI = new RuntimeVariableGUI();

	void RuntimeBoolGUI::Render()
	{
		ImGui::Checkbox(m_VariableName.c_str(), &m_VariableRef);
	}

	void RuntimeFloatGUI::Render()
	{
		ImGui::SliderFloat("", &m_VariableRef, m_RangeMin, m_RangeMax, (m_VariableName + " = %.3f").c_str());
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