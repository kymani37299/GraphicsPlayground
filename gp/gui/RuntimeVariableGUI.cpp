#include "RuntimeVariableGUI.h"

#include <imgui.h>

namespace GP
{
	RuntimeVariableGUI* g_RuntimeVariableGUI = new RuntimeVariableGUI();

	void RuntimeBoolGUI::Render()
	{
		ImGui::Checkbox(m_VariableName.c_str(), &m_VariableRef);
	}

	void RuntimeIntGUI::Render()
	{
		ImGui::InputInt(m_VariableName.c_str(), &m_VariableRef);
	}

	void RuntimeFloatGUI::Render()
	{
		ImGui::SliderFloat("", &m_VariableRef, m_RangeMin, m_RangeMax, (m_VariableName + " = %.3f").c_str());
	}

	void RuntimeSelectGUI::Render()
	{
		const char* currentItem = m_Labels[m_VariableRef].c_str();
		if (ImGui::BeginCombo(m_VariableName.c_str(), currentItem , 0))
		{
			size_t index = 0;
			for (const std::string& label : m_Labels)
			{
				const char* labelStr = label.c_str();
				const bool isSelected = (currentItem == labelStr);
				if (ImGui::Selectable(labelStr, isSelected))
					m_VariableRef = index;
				if(isSelected)
					ImGui::SetItemDefaultFocus();
				index++;
			}
			ImGui::EndCombo();
		}
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