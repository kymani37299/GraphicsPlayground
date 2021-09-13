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
	class GfxTexture;
	class GfxVertexBuffer;
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
	public:
		ENGINE_DLL virtual ~Scene();

		ENGINE_DLL SceneObject* Load(const std::string& path);

		inline const std::vector<SceneObject*>& GetObjects() const { return m_Objects; }

	private:
		std::vector<SceneObject*> m_Objects;
	};

}

#endif // SCENE_SUPPORT