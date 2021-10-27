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

    inline Mat4 SceneObject::GetTransformationMatrix(const Transform& transform)
    {
        // TODO : Rotation
        Mat4 model = glm::translate(MAT4_IDENTITY, transform.position);
        model = glm::scale(model, Vec3(transform.scale));
        return model;
    }

    SceneObject::SceneObject(Mesh* mesh, Material* material):
        m_Mesh(mesh),
        m_Material(material),
        m_InstanceBuffer(new GfxConstantBuffer<CBInstanceData>())
    {
        UpdateInstance();
    }

    SceneObject::~SceneObject()
    {
        delete m_Mesh;
        delete m_Material;
        delete m_InstanceBuffer;
    }

    void SceneObject::UpdateInstance()
    {
        Mat4 model = GetTransformationMatrix(m_Transform);
        static CBInstanceData instanceData = {};
        instanceData.model = model;
        m_InstanceBuffer->Upload(instanceData);
    }

    ///////////////////////////////////////
    //			Scene					//
    /////////////////////////////////////

    void Scene::Load(const std::string& path)
    {
        if (m_LoadingJob)
        {
            // For now we can load just one model at the time
            m_LoadingJob->WaitToLoad();
            delete m_LoadingJob;
        }
        m_LoadingJob = new SceneLoadingJob(this, path);
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