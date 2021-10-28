#pragma once

#include <string>

namespace GP
{
	class RuntimeValue
	{
	public:
		RuntimeValue(const std::string& name);
		virtual ~RuntimeValue() {}

	private:
		std::string& m_Name;
	};
}