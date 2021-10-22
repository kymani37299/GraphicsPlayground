#include "LoggerGUI.h"

#include <imgui.h>
#include <vector>
#include <string>

#include <debug/Logger.h>

namespace GP
{
    namespace
    {
        ImGuiTextBuffer     Buf;
        ImGuiTextFilter     Filter;
        ImVector<int>       LineOffsets;        // Index to lines offset
        bool                ScrollToBottom;
    }

	void LoggerGUI::Update(float dt)
	{
        std::vector<std::string>& pendingLogs = Logger::Get()->m_PendingConsoleLogs;
        for (const std::string& line : pendingLogs)
        {
            AddLine(line + "\n");
        }
        pendingLogs.clear();
        
	}

	void LoggerGUI::Render()
	{
        static bool active = true;
        ImGui::SetNextWindowSize(ImVec2(500, 400));
        ImGui::Begin("Console log", &active);

        if (ImGui::Button("Clear")) ClearLog();

        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);
        ImGui::Separator();
        ImGui::BeginChild("scrolling");
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
        if (copy) ImGui::LogToClipboard();

        if (Filter.IsActive())
        {
            const char* buf_begin = Buf.begin();
            const char* line = buf_begin;
            for (int line_no = 0; line != NULL; line_no++)
            {
                const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : NULL;
                if (Filter.PassFilter(line, line_end))
                    ImGui::TextUnformatted(line, line_end);
                line = line_end && line_end[1] ? line_end + 1 : NULL;
            }
        }
        else
        {
            ImGui::TextUnformatted(Buf.begin());
        }

        if (ScrollToBottom)
        {
            ImGui::SetScrollHereX(1.0f);
            ImGui::SetScrollHereY(1.0f);
        }

        ScrollToBottom = false;

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::End();
	}

    void LoggerGUI::ClearLog()
    {
        Buf.clear(); 
        LineOffsets.clear();
    }

    void LoggerGUI::AddLine(const std::string& line)
    {
        int old_size = Buf.size();
        Buf.append(line.c_str());
        for (int new_size = Buf.size(); old_size < new_size; old_size++)
        {
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size);
        }

        ScrollToBottom = true;
    }
}