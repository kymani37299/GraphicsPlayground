#pragma once

#include <string>
#include <vector>

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

	class RuntimeInt
	{
	public:
		GP_DLL RuntimeInt(const std::string& name, int initialvalue = 0);

		inline int GetValue() const { return m_Value; }

	private:
		int m_Value = 0;
	};

	class RuntimeFloat
	{
	public:
		GP_DLL RuntimeFloat(const std::string& name, float initialValue = 0.0f, float rangeMin = 0.0f, float rangeMax = 1.0f);

		inline float GetValue() const { return m_Value; }

	private:
		float m_Value = 0.0f;
	};

	class RuntimeSelect
	{
	public:
		GP_DLL RuntimeSelect(const std::string& name, std::vector<std::string> labels, unsigned int initialValue = 0);

		inline int GetValue() const { return m_Value; }
		inline std::string GetValueString() const { return m_Labels[m_Value]; }

	private:
		int m_Value = 0;
		std::vector<std::string> m_Labels;
	};
}