#pragma once

#include <string>
#include <stringapiset.h>

namespace GP
{
    namespace StringUtil
    {
        static std::wstring ToWideString(const std::string& s)
        {
            int len;
            int slength = (int)s.length() + 1;
            len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
            wchar_t* buf = new wchar_t[len];
            MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
            std::wstring r(buf);
            delete[] buf;
            return r;
        }

        static void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
            if (from.empty())
                return;
            size_t start_pos = 0;
            while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length();
            }
        }

        static bool Contains(const std::string& string, const std::string& param)
        {
            return string.find(param) != std::string::npos;
        }
    }
}