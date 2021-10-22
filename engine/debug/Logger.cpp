#include "Logger.h"

#include <iostream>

#include <windows.h>
#include <fcntl.h>
#include <io.h>

void OpenConsoleWindow()
{
    AllocConsole();

    HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
    FILE* hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;

    HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
    FILE* hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, NULL, _IONBF, 128);
    *stdin = *hf_in;
}

namespace GP
{
    Logger* Logger::s_Instance = nullptr;

    Logger* Logger::Get()
    {
        if (!s_Instance)
            s_Instance = new Logger();
        return s_Instance;
    }

    Logger::Logger()
    {
        // IO redirect not working :/
        //OpenConsoleWindow();  
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