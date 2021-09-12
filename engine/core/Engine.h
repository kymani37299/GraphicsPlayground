#pragma once

class Window;
class Renderer;

class Engine
{
public:
	Engine(Window* window);
	~Engine();

	void Run();

	inline bool IsFirstFrame() const { return m_FirstFrame; }

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