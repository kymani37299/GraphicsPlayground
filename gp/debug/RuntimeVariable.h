#pragma once

#include <string>

#include "Common.h"

namespace GP
{
	class RuntimeBool
	{
	public:
		GP_DLL RuntimeBool(const std::string& name, bool initialvalue = false);

		inline bool GetValue() const { return m_Value; }

	private:
		bool m_Value = false;
	};

	class RuntimeFloat
	{
	public:
		GP_DLL RuntimeFloat(const std::string& name, float initialValue = 0.0f, float rangeMin = 0.0f, float rangeMax = 1.0f);

		inline float GetValue() const { return m_Value; }

	private:
		float m_Value = 0.0f;
	};
}