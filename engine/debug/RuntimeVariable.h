#pragma once

#include <string>

#include "Common.h"

namespace GP
{
	class RuntimeFloatSlider
	{
	public:
		ENGINE_DLL RuntimeFloatSlider(const std::string& name, float initialValue = 0.0f, float rangeMin = 0.0f, float rangeMax = 1.0f);

		inline float GetValue() const { return m_Value; }

	private:
		float m_Value = 0.0f;
	};
}