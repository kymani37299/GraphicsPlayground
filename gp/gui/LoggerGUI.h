#pragma once

#include <string>

#include "gui/GUI.h"

namespace GP
{
	class LoggerGUI : public GUIElement
	{
	public:
		virtual void Update(float dt);
		virtual void Render();

	private:
		void ClearLog();
		void AddLine(const std::string& line);
	};
}