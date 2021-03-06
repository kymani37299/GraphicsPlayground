#pragma once

#include "Common.h"

#ifdef SCENE_SUPPORT

#include "core/Threads.h"
#include "gfx/GfxTransformations.h"

#include <algorithm>
#include <string>
#include <vector>

namespace GP
{
	template<typename T> class GfxConstantBuffer;
	class GfxTexture2D;
	template<typename T> class GfxVertexBuffer;
	class GfxIndexBuffer;
	class GfxContext;

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
		SceneObject(Mesh* mesh, Material* material);
		~SceneObject();

		inline Mesh* GetMesh() const { return m_Mesh; }
		inline Material* GetMaterial() const { return m_Material; }

		inline GfxConstantBuffer<Mat4>* GetTransformBuffer(GfxContext* context) { return m_Transform.GetBuffer(context); }
		inline Vec3 GetPosition() const { return m_Transform.GetPosition(); }
		inline Vec3 GetRotation() const { return m_Transform.GetRotation();  }
		inline Vec3 GetScale() const { return m_Transform.GetScale(); }

		inline void SetPostition(Vec3 position) { m_Transform.SetPosition(position); }
		inline void SetRotation(Vec3 rotation) { m_Transform.SetRotation(rotation);  }
		inline void SetScale(Vec3 scale) { m_Transform.SetScale(scale);  }

	private:
		Mesh* m_Mesh;
		Material* m_Material;
		ModelTransform m_Transform;
	};

	class Scene
	{
	public:
		Scene() {}
		GP_DLL virtual ~Scene();
		GP_DLL void Load(const std::string& path, Vec3 position = VEC3_ZERO, Vec3 scale = VEC3_ONE, Vec3 rotation = VEC3_ZERO);

		inline void AddSceneObjects(const std::vector<SceneObject*>& sceneObjects)
		{
			m_Objects.Lock();
			for (SceneObject* sceneObject : sceneObjects) m_Objects.Add(sceneObject);
			m_Objects.Unlock();
		}

		inline void AddSceneObject(SceneObject* sceneObject) 
		{ 
			m_Objects.Add(sceneObject);
		}

		template<typename F>
		void ForEveryObject(F& func)
		{
			m_Objects.ForEach(func);
		}

		template<typename F>
		void ForEveryOpaqueObject(F& func)
		{
			m_Objects.ForEach([&func](SceneObject* sceneObject) {
					if (!sceneObject->GetMaterial()->IsTransparent())
						func(sceneObject);
				});
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
			
			m_Objects.ForEach([&transparentObjects](SceneObject* sceneObject) {
				if (sceneObject->GetMaterial()->IsTransparent())
					transparentObjects.push_back(sceneObject);
				});
			
			std::sort(transparentObjects.begin(), transparentObjects.end(), comparator);
			for (SceneObject* sceneObject : transparentObjects) func(sceneObject);
		}

	private:
		MutexVector<SceneObject*> m_Objects;
	};

}

#endif // SCENE_SUPPORT