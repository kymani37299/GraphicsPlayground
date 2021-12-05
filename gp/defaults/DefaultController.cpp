#include "DefaultController.h"

#include "GP.h"

namespace GP
{
    void DefaultController::UpdateGameInput(float dt)
    {
        static const float MOUSE_SPEED = 80.0f;
        static const float MOVE_SPEED = 20.0f;

        static const float MAX_PITCH = 1.5f;
        static const float MIN_PITCH = -1.5f;

        static const Vec3 UP_DIR = Vec3(0.0f, 1.0f, 0.0f);
        static const Vec3 RIGHT_DIR = Vec3(1.0f, 0.0f, 0.0f);
        static const Vec3 FORWARD_DIR = Vec3(0.0f, 0.0f, -1.0f);

        const float dtSeconds = dt / 100.0f;

        Vec3 moveDir = VEC3_ZERO;
        Vec2 rotation = VEC3_ZERO;

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
            rotation.y -= 1.0f;
        }

        if (GP::Input::IsKeyPressed(VK_DOWN))
        {
            rotation.y += 1.0f;
        }

        if (GP::Input::IsKeyPressed(VK_LEFT))
        {
            rotation.x -= 1.0f;
        }

        if (GP::Input::IsKeyPressed(VK_RIGHT))
        {
            rotation.x += 1.0f;
        }

        if (GP::Input::IsKeyJustPressed('R'))
        {
            GP::ReloadShaders();
            CONSOLE_LOG("Shader reload finished!");
        }

        if (glm::length(moveDir) > 0.001f)
        {
            Vec3 cameraPos = m_Camera.GetPosition();
            cameraPos += m_Camera.RelativeToView(moveDir) * dtSeconds * MOVE_SPEED;
            m_Camera.SetPosition(cameraPos);
        }

        rotation *= dtSeconds * 0.005f;
        rotation += GP::Input::GetMouseDelta();
        if (glm::length(rotation) > 0.001f)
        {
            Vec3 cameraRot = m_Camera.GetRotation();
            cameraRot.y += rotation.x * dtSeconds * MOUSE_SPEED;
            cameraRot.x -= rotation.y * dtSeconds * MOUSE_SPEED;
            cameraRot.x = CLAMP(cameraRot.x, MIN_PITCH, MAX_PITCH);
            m_Camera.SetRotation(cameraRot);
        }
    }

	void DefaultController::UpdateInput(float dt)
	{
        static bool guiMode = false;

        if (GP::Input::IsKeyJustPressed(VK_ESCAPE))
        {
            GP::Shutdown();
            return;
        }

        if (GP::Input::IsKeyJustPressed(VK_TAB))
        {
            guiMode = !guiMode;
            GP::ShowCursor(guiMode);
        }
        
        if (!guiMode)
        {
            UpdateGameInput(dt);
        }

	}
}