#include "Logger.h"

#include <iostream>

#include <windows.h>
#include <fcntl.h>
#include <io.h>

namespace GP
{
    Logger* Logger::s_Instance = nullptr;

    Logger* Logger::Get()
    {
        if (!s_Instance)
            s_Instance = new Logger();
        return s_Instance;
    }

    void Logger::ConsoleLog(const std::string& msg)
    {
        m_PendingConsoleLogs.push_back(msg);
    }

    void Logger::PopupLog(const std::string& msg)
    {
        MessageBoxA(0, msg.c_str(), "Log error", MB_ICONERROR | MB_OK);
    }
}