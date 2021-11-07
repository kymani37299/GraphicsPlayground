#include "RuntimeVariable.h"

#include "gui/RuntimeVariableGUI.h"

namespace GP
{
	RuntimeBool::RuntimeBool(const std::string& name, bool initialvalue):
		m_Value(initialvalue)
	{
		ASSERT(g_RuntimeVariableGUI, "[RuntimeBool] RuntimeVariableGUI not initialized. Must call GP::Init before creating runtime variables");
		g_RuntimeVariableGUI->AddRuntimeVariable(new RuntimeBoolGUI(name, m_Value));
	}

	RuntimeFloat::RuntimeFloat(const std::string& name, float initialValue, float rangeMin, float rangeMax):
		m_Value(initialValue)
	{
		ASSERT(g_RuntimeVariableGUI, "[RuntimeFloatSlider] RuntimeVariableGUI not initialized. Must call GP::Init before creating runtime variables");
		g_RuntimeVariableGUI->AddRuntimeVariable(new RuntimeFloatGUI(name, m_Value, rangeMin, rangeMax));
	}
}