#include "DefaultController.h"

#include "Engine.h"

namespace GP
{
	void DefaultController::UpdateInput(float dt)
	{
        static const Vec3 UP_DIR = Vec3(0.0f, 1.0f, 0.0f);
        static const Vec3 RIGHT_DIR = Vec3(1.0f, 0.0f, 0.0f);
        static const Vec3 FORWARD_DIR = Vec3(0.0f, 0.0f, -1.0f);

        Vec3 moveDir = VEC3_ZERO;
        Vec3 rotation = VEC3_ZERO;

        if (GP::Input::IsKeyPressed(VK_ESCAPE))
        {
            GP::Shutdown();
            return;
        }

        if (GP::Input::IsKeyPressed('W'))
        {
            moveDir += FORWARD_DIR;
        }

        if (GP::Input::IsKeyPressed('S'))
        {
            moveDir -= FORWARD_DIR;
        }

        if (GP::Input::IsKeyPressed('A'))
        {
            moveDir -= RIGHT_DIR;
        }

        if (GP::Input::IsKeyPressed('D'))
        {
            moveDir += RIGHT_DIR;
        }

        if (GP::Input::IsKeyPressed('Q'))
        {
            moveDir += UP_DIR;
        }

        if (GP::Input::IsKeyPressed('E'))
        {
            moveDir -= UP_DIR;
        }

        if (GP::Input::IsKeyPressed(VK_UP))
        {
            rotation.x += 1.0f;
        }

        if (GP::Input::IsKeyPressed(VK_DOWN))
        {
            rotation.x -= 1.0f;
        }

        if (GP::Input::IsKeyPressed(VK_LEFT))
        {
            rotation.y -= 1.0f;
        }

        if (GP::Input::IsKeyPressed(VK_RIGHT))
        {
            rotation.y += 1.0f;
        }

        if (GP::Input::IsKeyPressed('R'))
        {
            GP::ReloadShaders();
        }

        if (glm::length(moveDir) > 0.001f)
        {
            Vec3 cameraPos = m_Camera.GetPosition();
            cameraPos += m_Camera.RelativeToView(moveDir);
            m_Camera.SetPosition(cameraPos);
        }
	}
}