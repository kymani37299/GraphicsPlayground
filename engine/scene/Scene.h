#pragma once

#include "Common.h"

#ifdef SCENE_SUPPORT

#include <string>
#include <vector>

namespace GP
{
	namespace SceneLoading
	{
		struct MaterialData;
		struct MeshData;
		struct ObjectData;
	}

	template<typename T> class GfxConstantBuffer;
	template<typename T> class GfxStructuredBuffer;
	class GfxTexture;
	class GfxVertexBuffer;
	class GfxIndexBuffer;

	///////////////////////////////////////
	//			DATA STRUCTS			//
	/////////////////////////////////////

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

	struct CBInstanceData
	{
		Mat4 model = MAT4_IDENTITY;
	};

	struct Transform
	{
		Vec3 position = VEC3_ZERO;
		Vec3 rotation = VEC3_ZERO;
		float scale = 1.0f;
	};

	///////////////////////////////////////
	//			Camera					//
	/////////////////////////////////////

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

	///////////////////////////////////////
	//			Scene					//
	/////////////////////////////////////

	class Material
	{
	public:
		Material(const SceneLoading::MaterialData& data);
		~Material();

		// TODO: Use default black texture instead
		inline GfxTexture* GetDiffuseTexture() const { return m_DiffuseTexture ? m_DiffuseTexture : nullptr; }
		inline GfxTexture* GetMetallicTexture() const { return m_MetallicTexture ? m_MetallicTexture : nullptr; }
		inline GfxTexture* GetRoughnessTexture() const { return m_RoughnessTexture ? m_RoughnessTexture : nullptr; }
		inline GfxTexture* GetAoTexture() const { return m_AoTexture ? m_AoTexture : nullptr; }

	private:
		GfxTexture* m_DiffuseTexture = nullptr;
		Vec3 m_DiffuseColor = VEC3_ZERO;

		GfxTexture* m_MetallicTexture = nullptr;
		float m_Metallic = 0.5f;

		GfxTexture* m_RoughnessTexture = nullptr;
		float m_Roughness = 0.5f;

		GfxTexture* m_AoTexture = nullptr;
		float m_Ao = 0.05f;
	};

	class Mesh
	{
	public:
		Mesh(const SceneLoading::MeshData& data);
		~Mesh();

		inline GfxVertexBuffer* GetVertexBuffer() const { return m_VertexBuffer; }
		inline GfxIndexBuffer* GetIndexBuffer() const { return m_IndexBuffer; }

	private:
		GfxVertexBuffer* m_VertexBuffer;
		GfxIndexBuffer* m_IndexBuffer;
	};

	class SceneObject
	{
	public:
		static inline Mat4 GetTransformationMatrix(const Transform& transform);

	public:
		SceneObject(const SceneLoading::ObjectData& data);
		~SceneObject();

		inline Mesh* GetMesh() const { return m_Mesh; }
		inline Material* GetMaterial() const { return m_Material; }
		inline GfxConstantBuffer<CBInstanceData>* GetInstanceBuffer() const { return m_InstanceBuffer; }

		inline void SetPostition(Vec3 position) { m_Transform.position = position; UpdateInstance(); }
		inline void SetRotation(Vec3 rotation) { m_Transform.rotation = rotation; UpdateInstance(); }
		inline void SetScale(float scale) { m_Transform.scale = scale; UpdateInstance(); }

	private:
		void UpdateInstance();

	private:
		Mesh* m_Mesh;
		Material* m_Material;
		Transform m_Transform;

		GfxConstantBuffer<CBInstanceData>* m_InstanceBuffer;
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
		SceneObject* m_PointLightObject = nullptr;
		Vec3 m_DirectionalLight = glm::normalize(Vec3(0.5, -0.5, 0.5));

		float m_PlayerSpeed = 0.1f;
		float m_PlayerMouseSensitivity = 0.01f;
	};

}

#endif // SCENE_SUPPORT