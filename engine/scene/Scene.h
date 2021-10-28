#pragma once

#include "Common.h"

#ifdef SCENE_SUPPORT

#include <string>
#include <vector>
#include <thread>
#include <mutex>

namespace GP
{
	template<typename T> class GfxConstantBuffer;
	class GfxTexture2D;
	template<typename T> class GfxVertexBuffer;
	class GfxIndexBuffer;

	class SceneLoadingJob;

	///////////////////////////////////////
	//			DATA STRUCTS			//
	/////////////////////////////////////

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
	//			Scene					//
	/////////////////////////////////////

	class Material
	{
	public:
		Material(GfxTexture2D* diffuseTexture):
			m_DiffuseTexture(diffuseTexture) {}
		~Material();

		// TODO: Use default black texture instead
		inline GfxTexture2D* GetDiffuseTexture() const { return m_DiffuseTexture ? m_DiffuseTexture : nullptr; }

	private:
		GfxTexture2D* m_DiffuseTexture = nullptr;
		Vec3 m_DiffuseColor = VEC3_ZERO;
	};

	class Mesh
	{
	public:
		Mesh(GfxVertexBuffer<Vec3>* positionBuffer, GfxVertexBuffer<Vec2>* uvBuffer, GfxVertexBuffer<Vec3>* normalBuffer, GfxVertexBuffer<Vec4>* tangentBuffer, GfxIndexBuffer* indexBuffer):
			m_PositionBuffer(positionBuffer),
			m_UVBuffer(uvBuffer),
			m_NormalBuffer(normalBuffer),
			m_TangentBuffer(tangentBuffer),
			m_IndexBuffer(indexBuffer) {}

		~Mesh();

		inline GfxVertexBuffer<Vec3>* GetPositionBuffer() const { return m_PositionBuffer; }
		inline GfxVertexBuffer<Vec2>* GetUVBuffer() const { return m_UVBuffer; }
		inline GfxVertexBuffer<Vec3>* GetNormalBuffer() const { return m_NormalBuffer; }
		inline GfxVertexBuffer<Vec4>* GetTangentBuffer() const { return m_TangentBuffer; }
		inline GfxIndexBuffer* GetIndexBuffer() const { return m_IndexBuffer; }

	private:
		GfxVertexBuffer<Vec3>* m_PositionBuffer;
		GfxVertexBuffer<Vec2>* m_UVBuffer;
		GfxVertexBuffer<Vec3>* m_NormalBuffer;
		GfxVertexBuffer<Vec4>* m_TangentBuffer;

		GfxIndexBuffer* m_IndexBuffer;
	};

	class SceneObject
	{
	public:
		static inline Mat4 GetTransformationMatrix(const Transform& transform);

	public:
		SceneObject(Mesh* mesh, Material* material);
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
	public:
		Scene() {}
		ENGINE_DLL virtual ~Scene();
		ENGINE_DLL void Load(const std::string& path, Vec3 position = VEC3_ZERO, Vec3 scale = VEC3_ONE, Vec3 rotation = VEC3_ZERO);

		inline void AddSceneObjects(const std::vector<SceneObject*>& sceneObjects)
		{
			m_ObjectsMutex.lock();
			for (SceneObject* sceneObject : sceneObjects) m_Objects.push_back(sceneObject);
			m_ObjectsMutex.unlock();
		}

		inline void AddSceneObject(SceneObject* sceneObject) 
		{ 
			m_ObjectsMutex.lock();
			m_Objects.push_back(sceneObject); 
			m_ObjectsMutex.unlock();
		}

		template<typename F>
		void ForEverySceneObject(F& func)
		{
			m_ObjectsMutex.lock();
			for (SceneObject* sceneObject : m_Objects) func(sceneObject);
			m_ObjectsMutex.unlock();
		}

	private:
		std::vector<SceneObject*> m_Objects;
		std::mutex m_ObjectsMutex; // TODO: Use concurrent structures instead of mutex
		SceneLoadingJob* m_LoadingJob = nullptr;
	};

}

#endif // SCENE_SUPPORT