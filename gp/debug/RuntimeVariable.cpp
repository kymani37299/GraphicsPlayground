#include "RuntimeVariable.h"

#include "gui/RuntimeVariableGUI.h"

namespace GP
{
	RuntimeFloatSlider::RuntimeFloatSlider(const std::string& name, float initialValue, float rangeMin, float rangeMax):
		m_Value(initialValue)
	{
		ASSERT(g_RuntimeVariableGUI, "[RuntimeFloatSlider] RuntimeVariableGUI not initialized. Must call GP::Init before creating runtime variables");
		g_RuntimeVariableGUI->AddRuntimeVariable(new RuntimeFloatSliderGUI(name, m_Value, rangeMin, rangeMax));
	}
}