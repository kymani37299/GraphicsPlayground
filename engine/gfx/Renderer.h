#pragma once

#include <vector>

#include "gfx/device/DxDevice.h"

class RenderPass;
class Window;
class Scene;

class Renderer
{
public:
	Renderer(Window* window);
	~Renderer();

	void Update(float dt);
	bool RenderIfShould();

	void ReloadShaders();

private:
	void RenderFrame();

private:
	bool m_ShouldRender = true;

	DxDevice m_Device;
	Scene* m_Scene = nullptr;
	std::vector<RenderPass*> m_Schedule;
};