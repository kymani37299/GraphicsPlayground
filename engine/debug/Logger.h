#pragma once

#include <string>
#include <vector>

#include "Common.h"

namespace GP
{
	class LoggerGUI;

	class Logger
	{
		friend class LoggerGUI;

	private:
		static Logger* s_Instance;
		Logger() {}

	public:
		ENGINE_DLL static Logger* Get();

	public:
		ENGINE_DLL void ConsoleLog(const std::string& message);
		ENGINE_DLL void PopupLog(const std::string& message);

	private:
		std::vector<std::string> m_PendingConsoleLogs;
	};
}