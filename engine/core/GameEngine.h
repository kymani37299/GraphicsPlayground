#pragma once

#include "core/Controller.h"

namespace GP
{
	class Window;
	class Renderer;

	class GameEngine
	{
	public:
		GameEngine(Window* window);
		~GameEngine();

		void Run();

		inline bool IsFirstFrame() const { return m_FirstFrame; }
		inline Renderer* GetRenderer() const { return m_Renderer; }
		inline void SetController(Controller* controller) { delete m_Controller; m_Controller = controller; }

	private:
		void GameLoop();
		void UpdateDT();

	private:
		float m_DT = 0.0f; // Miliseconds
		bool m_FirstFrame = true;

		Window* m_Window;
		Renderer* m_Renderer;
		Controller* m_Controller;
	};
}

