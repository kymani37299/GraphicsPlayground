#include "GfxTransformations.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/perpendicular.hpp>

#include "core/GfxCore.h"

namespace GP
{
    namespace 
    {
        static inline void RotToAxis(Vec3 rot, Vec3& forward, Vec3& up, Vec3& right)
        {
            // TODO: Calculate up based on roll
            up = Vec3(0.0f, 1.0f, 0.0f);
            forward = Vec3((float)(glm::cos(rot.y) * glm::cos(rot.x)), (float)(glm::sin(rot.x)), (float)(glm::sin(rot.y) * glm::cos(rot.x)));
            right = glm::perp(forward, up);
        }

        static inline void AxisToRot(Vec3& rot, Vec3 forward, Vec3 up, Vec3 right)
        {
            float roll = 0;
            float yaw = 0;
            float pitch = glm::asin(-forward.z);
            float cosPitch = glm::sqrt(1 - forward.z * forward.z);

            if (cosPitch == 0 || glm::abs(forward.z) >= 1)
            {
                if (pitch > 0)
                {
                    yaw = 0;
                    roll = glm::atan(-up.y, -up.x) + 180;
                }
                else
                {
                    yaw = 0;
                    roll = -glm::atan(up.y, up.x) + 180;
                }
            }
            else
            {
                float cosYaw = forward.x / cosPitch;
                float sinYaw = forward.y / cosPitch;
                yaw = glm::atan(sinYaw, cosYaw);

                float cosRoll = up.z / cosPitch;
                float sinRoll;
                if (glm::abs(cosYaw) < glm::abs(sinYaw))
                {
                    sinRoll = -(up.x + forward.z * cosRoll * cosYaw) / sinYaw;
                }
                else
                {
                    sinRoll = (up.y + forward.z * cosRoll * sinYaw) / cosYaw;
                }
                roll = glm::atan(sinRoll, cosRoll);
            }

            if (yaw < 0)
                yaw += 360;
            else if (yaw >= 360)
                yaw -= 360;

            if (pitch < 0)
                pitch += 360;
            else if (pitch >= 360)
                pitch -= 360;

            if (roll < 0)
                roll += 360;
            else if (roll >= 360)
                roll -= 360;

            rot.x = glm::radians(pitch);
            rot.y = glm::radians(yaw);
            rot.z = glm::radians(roll);
        }
    }

    ///////////////////////////////////////
    //			Camera  		        //
    /////////////////////////////////////

    Camera::Camera()
    {
        m_Buffer = new GfxConstantBuffer<CBCamera>();

        m_Data.projection = glm::perspective(glm::radians(45.0f), 18.0f / 10.0f, 0.1f, 10000.0f);
        m_Data.projectionInv = glm::inverse(m_Data.projection);
        UpdateView();
    }

    Camera::~Camera()
    {
        delete m_Buffer;
    }

    void Camera::SetPosition(const Vec3 position)
    {
        m_Position = position;
        PositionChanged();
    }

    void Camera::LookAt(const Vec3& point)
    {
        const Vec3 dir = point - m_Position;
        if (dir != VEC3_ZERO)
        {
            m_Forward = glm::normalize(dir);
            m_Right = glm::perp(m_Forward, m_Up);
            AxisChanged();
        }
    }

    void Camera::SetRotation(const Vec3 rotation)
    {
        m_Rotation = rotation;
        RotationChanged();
    }

    void Camera::PositionChanged()
    {
        m_Data.position = m_Position;
        UpdateView();
    }

    void Camera::RotationChanged()
    {
        RotToAxis(m_Rotation, m_Forward, m_Up, m_Right);
        UpdateView();
    }

    void Camera::AxisChanged()
    {
        AxisToRot(m_Rotation, m_Forward, m_Up, m_Right);
        UpdateView();
    }

    void Camera::UpdateView()
    {
        m_Data.view = glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
        m_Data.viewInv = glm::inverse(m_Data.view);
        m_Buffer->Upload(m_Data);
    }

    ///////////////////////////////////////
    //			ModelTransform  		//
    /////////////////////////////////////

    ModelTransform::ModelTransform()
    {
        m_Buffer = new GfxConstantBuffer<Mat4>();
        UpdateBuffer();
    }

    ModelTransform::~ModelTransform()
    {
        delete m_Buffer;
    }
    
    void ModelTransform::UpdateBuffer()
    {
        Vec3 forward, up, right;
        RotToAxis(m_Rotation, forward, up, right);

        m_Data = glm::translate(MAT4_IDENTITY, m_Position);
        m_Data = glm::scale(m_Data, m_Scale);
        //m_Data = m_Data * glm::lookAt(m_Position, m_Position + forward, up); TODO: Enable rotation

        m_Buffer->Upload(m_Data);
    }
}