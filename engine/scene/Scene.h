#pragma once

#include <string>
#include <vector>

#include "Common.h"

namespace GP
{

	template<typename T> class GfxConstantBuffer;
	template<typename T> class GfxStructuredBuffer;

	class SceneObject;

	struct CBCamera
	{
		Mat4 view;
		Mat4 projection;
		Vec3 position;
		Mat4 viewInv;
		Mat4 projectionInv;
	};

	struct CBEnvironment
	{
		alignas(16) Vec3 directionalLight;
		unsigned int numPointLights;
	};

	struct PointLight
	{
		Vec3 position;
		Vec3 color;
	};

	class Camera
	{
	public:
		Camera();

		void SetPosition(const Vec3 position);
		void SetRotation(const Vec3 rotation);
		void LookAt(const Vec3& point);

		inline const CBCamera& GetData() const { return m_Data; }
		inline Vec3 GetPosition() const { return m_Position; }
		inline Vec3 GetRotation() const { return m_Rotation; }

		inline Vec3 RelativeToView(const Vec3 direction) const
		{
			Vec4 dir = Vec4(direction.x, direction.y, direction.z, 0.0f);
			Vec4 relativeDir = dir * m_Data.view;
			return Vec3(relativeDir.x, relativeDir.y, relativeDir.z);
		}

	private:
		void PositionChanged();
		void RotationChanged();
		void AxisChanged();
		void UpdateView();

	private:
		CBCamera m_Data;
		Vec3 m_Position = VEC3_ZERO;
		Vec3 m_Rotation = VEC3_ZERO; // (pitch,yaw,roll)

		Vec3 m_Forward = { 0.0f,0.0f, -1.0f };
		Vec3 m_Right = { 1.0f,0.0f, 0.0f };
		Vec3 m_Up = { 0.0f,1.0f,0.0f };
	};

	class Scene
	{
		static constexpr unsigned int MAX_POINT_LIGHTS = 16;

	public:
		ENGINE_DLL Scene();
		ENGINE_DLL ~Scene();

		ENGINE_DLL void Init();
		ENGINE_DLL SceneObject* Load(const std::string& path);

		inline const std::vector<PointLight>& GetPointLights() const { return m_PointLights; }
		inline Vec3 GetDirectionalLight() const { return m_DirectionalLight; }
		inline const std::vector<SceneObject*>& GetObjects() const { return m_Objects; }
		inline SceneObject* GetPointLightObject() const { return m_PointLightObject; }
		inline const Camera& GetCamera() const { return m_Camera; }

		ENGINE_DLL void MovePlayer(Vec3 direction); // Direction should be normalized
		ENGINE_DLL void RotatePlayer(Vec3 rotation);

		inline GfxConstantBuffer<CBCamera>* GetCameraBuffer() const { return m_CameraBuffer; }
		inline GfxConstantBuffer<CBEnvironment>* GetEnvironmentBuffer() const { return m_EnvironmentBuffer; }
		inline GfxStructuredBuffer<PointLight>* GetPointLightBuffer() const { return m_PointLightBuffer; }

		ENGINE_DLL void AddPointLight(const PointLight& pointLight);

	private:
		Camera m_Camera;

		GfxConstantBuffer<CBCamera>* m_CameraBuffer;
		GfxConstantBuffer<CBEnvironment>* m_EnvironmentBuffer;
		GfxStructuredBuffer<PointLight>* m_PointLightBuffer;

		std::vector<PointLight> m_PointLights;
		std::vector<SceneObject*> m_Objects;
		SceneObject* m_PointLightObject;
		Vec3 m_DirectionalLight = glm::normalize(Vec3(0.5, -0.5, 0.5));

		float m_PlayerSpeed = 0.1f;
		float m_PlayerMouseSensitivity = 0.01f;
	};

}