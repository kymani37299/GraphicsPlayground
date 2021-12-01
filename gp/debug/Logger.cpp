#include "Logger.h"

#include <iostream>

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <fstream>

namespace GP
{
    static constexpr const char* FILE_PATH = "Log.txt";
    static std::ofstream s_LogFile;

    Logger* Logger::s_Instance = nullptr;

    Logger* Logger::Get()
    {
        if (!s_Instance)
        {
            s_LogFile.open(FILE_PATH, std::ios::out | std::ios::trunc);
            s_LogFile << "File log started: " << std::endl; // TODO: Add current date and time
            s_LogFile.close();
            s_Instance = new Logger();
        }
            
        return s_Instance;
    }

    void Logger::DispatchLogs()
    {
        if (!m_PendingFileLogs.Empty())
        {
            s_LogFile.open(FILE_PATH, std::ios::app);
            m_PendingFileLogs.ForEach([](const std::string& msg)
                {
                    s_LogFile << msg << std::endl;
                });
            s_LogFile.close();
        }

        if (!m_PendingPopupLogs.Empty())
        {
            std::string popupText = "";
            m_PendingPopupLogs.ForEach([&popupText](const std::string& msg)
                {
                    popupText += msg + "\n";
                });
            MessageBoxA(0, popupText.c_str(), "Log error", MB_ICONERROR | MB_OK);
        }
    }
}