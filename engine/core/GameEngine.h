#pragma once

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

	private:
		void GameLoop();

		void UpdateDT();
		void UpdateInput();

	private:
		float m_DT = 0.0f;
		bool m_FirstFrame = true;

		Window* m_Window;
		Renderer* m_Renderer;
	};
}

