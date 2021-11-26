#pragma once

#include "Common.h"

#ifdef SCENE_SUPPORT

#include <string>
#include <thread>

#include "util/PathUtil.h"
#include "core/Loading.h"

struct cgltf_primitive;
struct cgltf_material;

namespace GP
{
	class Scene;
	class SceneObject;
	class Mesh;
	class Material;

	class SceneLoadingTask : public LoadingTask
	{
		static constexpr unsigned int BATCH_SIZE = 4;
	public:
		SceneLoadingTask(Scene* scene, const std::string& path, Vec3 position, Vec3 scale, Vec3 rotation):
			m_Scene(scene),
			m_Path(path),
			m_FolderPath(PathUtil::GetPathWitoutFile(path)),
			m_ScenePosition(position),
			m_SceneScale(scale),
			m_SceneRotation(rotation)
		{
			const std::string& ext = PathUtil::GetFileExtension(m_Path);
			ASSERT(ext == "gltf", "[SceneLoading] For now we only support glTF 3D format.");
		}

		void Run(GfxContext* context) override { LoadScene(context); }

	private:
		void LoadScene(GfxContext* context);
		SceneObject* LoadSceneObject(cgltf_primitive* meshData);
		Mesh* LoadMesh(cgltf_primitive* mesh);
		Material* LoadMaterial(cgltf_material* materialData);

	private:
		Scene* m_Scene;
		Vec3 m_ScenePosition;
		Vec3 m_SceneScale;
		Vec3 m_SceneRotation;
		std::string m_Path;
		std::string m_FolderPath;
	};


}

#endif // SCENE_SUPPORT
