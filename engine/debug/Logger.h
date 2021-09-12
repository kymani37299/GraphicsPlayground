#pragma once

#include "Common.h"

#include <string>

#define LOG(X) ::GP::Logger::Get()->ConsoleLog(X)
#define POPUP(X) ::GP::Logger::Get()->PopupLog(X)

namespace GP
{
	class Logger
	{
	private:
		static Logger* s_Instance;
		Logger();

	public:
		static Logger* Get();

	public:
		void ConsoleLog(const std::string& message);
		void PopupLog(const std::string& message);
	};
}