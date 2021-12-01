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
		GP_DLL static Logger* Get();

	public:
		inline void FileLog(const std::string& msg)
		{
			m_PendingFileLogs.push_back(msg);
		}

		inline void ConsoleLog(const std::string& msg)
		{
			m_PendingConsoleLogs.push_back(msg);
		}

		inline void PopupLog(const std::string& msg)
		{
			m_PendingPopupLogs.push_back(msg);
		}

		// TODO: Dispatch logs in another thread
		GP_DLL void DispatchLogs();

	private:
		std::vector<std::string> m_PendingConsoleLogs;
		std::vector<std::string> m_PendingFileLogs;
		std::vector<std::string> m_PendingPopupLogs;
	};
}