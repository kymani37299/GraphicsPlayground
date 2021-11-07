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

	RuntimeInt::RuntimeInt(const std::string& name, int initialvalue):
		m_Value(initialvalue)
	{
		ASSERT(g_RuntimeVariableGUI, "[RuntimeInt] RuntimeVariableGUI not initialized. Must call GP::Init before creating runtime variables");
		g_RuntimeVariableGUI->AddRuntimeVariable(new RuntimeIntGUI(name, m_Value));
	}

	RuntimeFloat::RuntimeFloat(const std::string& name, float initialValue, float rangeMin, float rangeMax):
		m_Value(initialValue)
	{
		ASSERT(g_RuntimeVariableGUI, "[RuntimeFloat] RuntimeVariableGUI not initialized. Must call GP::Init before creating runtime variables");
		g_RuntimeVariableGUI->AddRuntimeVariable(new RuntimeFloatGUI(name, m_Value, rangeMin, rangeMax));
	}

	RuntimeSelect::RuntimeSelect(const std::string& name, std::vector<std::string> labels, unsigned int initialValue):
		m_Value(initialValue < 0 || initialValue >= labels.size() ? 0 : initialValue),
		m_Labels(labels)
	{
		ASSERT(g_RuntimeVariableGUI, "[RuntimeSelect] RuntimeVariableGUI not initialized. Must call GP::Init before creating runtime variables");
		g_RuntimeVariableGUI->AddRuntimeVariable(new RuntimeSelectGUI(name, labels, m_Value));

	}
}