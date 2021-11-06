#include "Scene.h"

#ifdef SCENE_SUPPORT

#include <thread>
#include <glm/gtc/matrix_transform.hpp>

#include "scene/SceneLoading.h"

#include "gfx/GfxCore.h"
#include "gfx/GfxBuffers.h"
#include "gfx/GfxTexture.h"

namespace GP
{
    ///////////////////////////////////////
    //			Material                //
    /////////////////////////////////////

    Material::~Material()
    {
        SAFE_DELETE(m_DiffuseTexture);
    }

    ///////////////////////////////////////
    //			Mesh                    //
    /////////////////////////////////////

    Mesh::~Mesh()
    {
        delete m_IndexBuffer;
        delete m_PositionBuffer;
        delete m_UVBuffer;
        delete m_NormalBuffer;
        delete m_TangentBuffer;
    }

    ///////////////////////////////////////
    //			SceneObject             //
    /////////////////////////////////////

    SceneObject::SceneObject(Mesh* mesh, Material* material):
        m_Mesh(mesh),
        m_Material(material)
    { }

    SceneObject::~SceneObject()
    {
        delete m_Mesh;
        delete m_Material;
    }

    ///////////////////////////////////////
    //			Scene					//
    /////////////////////////////////////

    void Scene::Load(const std::string& path, Vec3 position, Vec3 scale, Vec3 rotation)
    {
        m_LoadingJob = new SceneLoadingJob(this, path, position, scale, rotation, m_LoadingJob);
        m_LoadingJob->Run();
    }

    Scene::~Scene()
    {
        SAFE_DELETE(m_LoadingJob);
        for (SceneObject* sceneObject : m_Objects)
        {
            delete sceneObject;
        }
        m_Objects.clear();
    }
}

#endif // SCENE_SUPPORT