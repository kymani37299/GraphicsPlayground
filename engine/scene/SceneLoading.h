#pragma once

#include "Common.h"

#ifdef SCENE_SUPPORT

#include <string>
#include <thread>

#include "util/PathUtil.h"

struct cgltf_primitive;
struct cgltf_material;

namespace GP
{
	class Scene;
	class SceneObject;
	class Mesh;
	class Material;

	class SceneLoadingJob
	{
	public:
		SceneLoadingJob(Scene* scene, const std::string& path) :
			m_Scene(scene),
			m_Path(path),
			m_FolderPath(PathUtil::GetPathWitoutFile(path)) 
		{
			const std::string& ext = PathUtil::GetFileExtension(m_Path);
			ASSERT(ext == "gltf", "[SceneLoading] For now we only support giTF 3D format.");
		}

		void Run()
		{
			ASSERT(!m_Thread, "[SceneLoadingJob] Trying to run job that is already running!");
			m_Thread = new std::thread(&SceneLoadingJob::LoadScene, this);
		}

		void WaitToLoad()
		{
			ASSERT(m_Thread, "[SceneLoadingJob] Waiting for nonexistent job!");
			m_Thread->join();
		}

		~SceneLoadingJob()
		{
			if (m_Thread)
			{
				if(m_Thread->joinable()) m_Thread->join();
				delete m_Thread;
				m_Thread = nullptr;
			}
		}

	private:
		void LoadScene();
		SceneObject* LoadSceneObject(cgltf_primitive* meshData);
		Mesh* LoadMesh(cgltf_primitive* mesh);
		Material* LoadMaterial(cgltf_material* materialData);

	private:
		Scene* m_Scene;
		std::string m_Path;
		std::string m_FolderPath;
		std::thread* m_Thread = nullptr;
	};


}

#endif // SCENE_SUPPORT
