#include "Scene.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/perpendicular.hpp"

#include "core/GfxCore.h"
#include "util/ModelLoading.h"

#include "scene/SceneObject.h"

namespace GP
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

    inline static Mat4 OrthoProjection(float height, float width, float nearPlane, float farPlane)
    {
        const float h = height / 2.0f;
        const float w = width / 2.0f;
        return Mat4(glm::ortho(-w, w, -h, h, nearPlane, farPlane));
    }

    inline static Mat4 PerspectiveProjection(float fov, float aspect, float nearPlane, float farPlane)
    {
        return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    }

    Camera::Camera()
    {
        m_Data.projection = PerspectiveProjection(45.0f, 18.0f / 10.0f, 0.1f, 100.0f);
        m_Data.projectionInv = glm::inverse(m_Data.projection);
        UpdateView();
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
        m_Data.view = Mat4(glm::lookAt(m_Position, m_Position + m_Forward, m_Up));
        m_Data.viewInv = glm::inverse(m_Data.view);
    }

    Scene::Scene()
    {
        m_Camera.SetPosition({ -5.0f, 5.0f, -5.0f });
        m_Camera.LookAt({ 0.1f,0.1f,0.1f });
    }

    Scene::~Scene()
    {
        for (SceneObject* sceneObject : m_Objects)
        {
            delete sceneObject;
        }
        m_Objects.clear();

        SAFE_DELETE(m_PointLightObject);
        SAFE_DELETE(m_CameraBuffer);
        SAFE_DELETE(m_EnvironmentBuffer);
        SAFE_DELETE(m_PointLightBuffer);
    }

    void Scene::Init()
    {
        m_CameraBuffer = new GfxConstantBuffer<CBCamera>();
        m_CameraBuffer->Upload(m_Camera.GetData());

        m_EnvironmentBuffer = new GfxConstantBuffer<CBEnvironment>();
        m_PointLightBuffer = new GfxStructuredBuffer<PointLight>(MAX_POINT_LIGHTS);

        //{
        //    using namespace ModelLoading;
        //    SceneData* sphereData = LoadScene("res/Defaults/sphere.obj");
        //    ASSERT(sphereData->numObjects > 0, "Failed loading default sphere!");
        //    ObjectData* sphereObj = sphereData->pObjects[0];
        //    m_PointLightObject = new SceneObject(*sphereObj);
        //    m_PointLightObject->SetScale(0.1f);
        //    FreeScene(sphereData);
        //}
    }

    SceneObject* Scene::Load(const std::string& path)
    {
        using namespace ModelLoading;

        SceneData* sceneData = LoadScene(path.c_str());
        ASSERT(sceneData->numObjects > 0, "Loaded scene with 0 objects");

        SceneObject* firstObject = nullptr;
        for (size_t i = 0; i < sceneData->numObjects; i++)
        {
            SceneObject* newObject = new SceneObject(*sceneData->pObjects[i]);
            if (i == 0) firstObject = newObject;
            m_Objects.push_back(newObject);
        }

        for (size_t i = 0; i < sceneData->numLights; i++)
        {
            LightData* light = sceneData->pLights[i];
            if (light->type != LightType::Point) continue; // For now only supporting Point light

            PointLight pointLight = {};
            pointLight.position = light->position;
            pointLight.color = light->color;
            AddPointLight(pointLight);
        }

        FreeScene(sceneData);

        return firstObject;
    }

    void Scene::MovePlayer(Vec3 direction)
    {
        if (glm::length(direction) < 0.001f) return;

        Vec3 cameraPos = m_Camera.GetPosition();
        cameraPos += m_Camera.RelativeToView(direction) * m_PlayerSpeed;
        m_Camera.SetPosition(cameraPos);

        m_CameraBuffer->Upload(m_Camera.GetData());
    }

    void Scene::RotatePlayer(Vec3 rotation)
    {
        if (glm::length(rotation) < 0.001f) return;

        Vec3 cameraRot = m_Camera.GetRotation();
        cameraRot += rotation * m_PlayerMouseSensitivity;
        m_Camera.SetRotation(cameraRot);

        m_CameraBuffer->Upload(m_Camera.GetData());
    }

    void Scene::AddPointLight(const PointLight& pointLight)
    {
        ASSERT(m_PointLights.size() < MAX_POINT_LIGHTS, "There are more point lights in scene than supported!");
        m_PointLights.push_back(pointLight);
        m_PointLightBuffer->Upload(pointLight, m_PointLights.size() - 1);

        static CBEnvironment env;
        env.directionalLight = m_DirectionalLight;
        env.numPointLights = m_PointLights.size();
        m_EnvironmentBuffer->Upload(env);
    }
}

