#pragma once

#include <vector>
#include <string>

#include "Common.h"
#include "gui/GUI.h"

namespace GP
{
	class RuntimeVariableGUI;
	extern RuntimeVariableGUI* g_RuntimeVariableGUI;

	class RuntimeVariableGUIElement : public GUIElement 
	{
	public:
		void Update(float dt) { }
	};

	class RuntimeFloatSliderGUI : public RuntimeVariableGUIElement
	{
	public:
		RuntimeFloatSliderGUI(const std::string& name, float& variableRef, float rangeMin, float rangeMax) :
			m_VariableName(name),
			m_VariableRef(variableRef),
			m_RangeMin(rangeMin),
			m_RangeMax(rangeMax) {}

		void Render();

	private:
		std::string m_VariableName;
		float& m_VariableRef;
		float m_RangeMin;
		float m_RangeMax;
	};

	class RuntimeVariableGUI : public GUIElement
	{
	public:
		RuntimeVariableGUI::RuntimeVariableGUI()
		{
			ASSERT(!g_RuntimeVariableGUI, "[RuntimeVariableGUI] There is already an instance of this class");
		}

		void Update(float dt) {}
		void Render();

		inline void AddRuntimeVariable(RuntimeVariableGUIElement* runtimeVariable) { m_RuntimeVariables.push_back(runtimeVariable); }

	private:
		std::vector<RuntimeVariableGUIElement*> m_RuntimeVariables;
	};
}
