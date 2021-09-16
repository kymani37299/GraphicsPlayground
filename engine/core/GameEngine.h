#pragma once

#include "core/Controller.h"

namespace GP
{
	class Renderer;

	class GameEngine
	{
	public:
		GameEngine();
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

		Renderer* m_Renderer;
		Controller* m_Controller;
	};
}

