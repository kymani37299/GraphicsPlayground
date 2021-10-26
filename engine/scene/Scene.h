#pragma once

#include "Common.h"

#ifdef SCENE_SUPPORT

#include <string>
#include <vector>

namespace GP
{
	template<typename T> class GfxConstantBuffer;
	class GfxTexture2D;
	template<typename T> class GfxVertexBuffer;
	class GfxIndexBuffer;

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
		Scene(std::vector<SceneObject*> objects) :
			m_Objects(objects) {}
		ENGINE_DLL virtual ~Scene();

		inline const std::vector<SceneObject*>& GetObjects() const { return m_Objects; }

	private:
		std::vector<SceneObject*> m_Objects;
	};

}

#endif // SCENE_SUPPORT