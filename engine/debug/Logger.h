#pragma once

#include <string>
#include <vector>

#include "Common.h"

#undef ENGINE_DLL // TODO: Find out why the fuck ENGINE_DLL isn't defined by the Common.h
#ifdef ENGINE
#define ENGINE_DLL __declspec(dllexport)
#else
#define ENGINE_DLL __declspec(dllimport)
#endif // ENGINE

namespace GP
{
	class LoggerGUI;

	class Logger
	{
		friend class LoggerGUI;

	private:
		static Logger* s_Instance;
		ENGINE_DLL Logger();

	public:
		ENGINE_DLL static Logger* Get();

	public:
		ENGINE_DLL void ConsoleLog(const std::string& message);
		ENGINE_DLL void PopupLog(const std::string& message);

	private:
		std::vector<std::string> m_PendingConsoleLogs;
	};
}