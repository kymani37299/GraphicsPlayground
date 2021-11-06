#pragma once

#include "Common.h"

#ifdef SCENE_SUPPORT

#include <algorithm>
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
		Material(bool transparent, GfxTexture2D* diffuseTexture):
			m_Transparent(transparent),
			m_DiffuseTexture(diffuseTexture) {}
		~Material();

		inline bool IsTransparent() const { return m_Transparent; }
		inline GfxTexture2D* GetDiffuseTexture() const { return m_DiffuseTexture; }

	private:
		bool m_Transparent = false;
		GfxTexture2D* m_DiffuseTexture = nullptr;
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

		inline Vec3 GetPosition() const { return m_Transform.position; }
		inline Vec3 GetRotation() const { return m_Transform.rotation; }
		inline float GetScale() const { return m_Transform.scale; }

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
		void ForEveryObject(F& func)
		{
			m_ObjectsMutex.lock();
			for (SceneObject* sceneObject : m_Objects) func(sceneObject);
			m_ObjectsMutex.unlock();
		}

		template<typename F>
		void ForEveryOpaqueObject(F& func)
		{
			m_ObjectsMutex.lock();
			for (SceneObject* sceneObject : m_Objects)
			{
				if (!sceneObject->GetMaterial()->IsTransparent())
					func(sceneObject);
			}
			m_ObjectsMutex.unlock();
		}

		template<typename F>
		void ForEveryTransparentObjectSorted(Vec3 cameraPos, F& func)
		{
			const auto comparator = [cameraPos](SceneObject* a, SceneObject* b) 
			{
				float distA = glm::length(a->GetPosition() - cameraPos);
				float distB = glm::length(b->GetPosition() - cameraPos);
				return distA > distB;
			};
			std::vector<SceneObject*> transparentObjects; 
			
			m_ObjectsMutex.lock();
			for (SceneObject* sceneObject : m_Objects)
			{
				if (sceneObject->GetMaterial()->IsTransparent())
					transparentObjects.push_back(sceneObject);

			}
			m_ObjectsMutex.unlock();
			
			std::sort(transparentObjects.begin(), transparentObjects.end(), comparator);
			for (SceneObject* sceneObject : transparentObjects) func(sceneObject);
		}

	private:
		std::vector<SceneObject*> m_Objects;
		std::mutex m_ObjectsMutex; // TODO: Use concurrent structures instead of mutex
		SceneLoadingJob* m_LoadingJob = nullptr;
	};

}

#endif // SCENE_SUPPORT