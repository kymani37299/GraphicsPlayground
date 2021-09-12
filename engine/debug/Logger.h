#pragma once

#include <string>

#define LOG(X) Logger::Get()->ConsoleLog(X)
#define POPUP(X) Logger::Get()->PopupLog(X)

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