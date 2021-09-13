#include "Scene.h"

#ifdef SCENE_SUPPORT

#include <glm/gtc/matrix_transform.hpp>

#include "core/GfxCore.h"
#include "scene/SceneLoading.h"

namespace GP
{
    ///////////////////////////////////////
    //			Material                //
    /////////////////////////////////////

    inline GfxTexture* LoadMaterialTexture(SceneLoading::TextureData* textureData)
    {
        if (!textureData) return nullptr;

        std::vector<const void*> texData;
        texData.push_back(textureData->pData);

        TextureDesc texDesc = {};
        texDesc.texData = texData;
        texDesc.width = textureData->width;
        texDesc.height = textureData->height;

        return new GfxTexture(texDesc);
    }

    Material::Material(const SceneLoading::MaterialData& data) :
        m_DiffuseColor(data.diffuse),
        m_Ao(data.ao),
        m_Metallic(data.metallic),
        m_Roughness(data.roughness)
    {
        m_DiffuseTexture = LoadMaterialTexture(data.diffuseMap);
        m_MetallicTexture = LoadMaterialTexture(data.metallicMap);
        m_RoughnessTexture = LoadMaterialTexture(data.roughnessMap);
        m_AoTexture = LoadMaterialTexture(data.aoMap);
    }

    Material::~Material()
    {
        SAFE_DELETE(m_DiffuseTexture);
    }

    ///////////////////////////////////////
    //			Mesh                    //
    /////////////////////////////////////

    Mesh::Mesh(const SceneLoading::MeshData& data)
    {
        VertexBufferData vertexData = {};
        vertexData.pData = data.pVertices;
        vertexData.stride = SceneLoading::MeshVertex::GetStride();
        vertexData.numBytes = sizeof(SceneLoading::MeshVertex) * data.numVertices;

        m_VertexBuffer = new GfxVertexBuffer(vertexData);
        m_IndexBuffer = new GfxIndexBuffer(data.pIndices, data.numIndices);
    }

    Mesh::~Mesh()
    {
        delete m_IndexBuffer;
        delete m_VertexBuffer;
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

    SceneObject::SceneObject(const SceneLoading::ObjectData& data)
    {
        m_Mesh = new Mesh(*data.mesh);
        m_Material = new Material(data.material);
        m_InstanceBuffer = new GfxConstantBuffer<CBInstanceData>();
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

    Scene::~Scene()
    {
        for (SceneObject* sceneObject : m_Objects)
        {
            delete sceneObject;
        }
        m_Objects.clear();
    }

    SceneObject* Scene::Load(const std::string& path)
    {
        using namespace SceneLoading;

        SceneData* sceneData = LoadScene(path.c_str());
        ASSERT(sceneData->numObjects > 0, "Loaded scene with 0 objects");

        SceneObject* firstObject = nullptr;
        for (size_t i = 0; i < sceneData->numObjects; i++)
        {
            SceneObject* newObject = new SceneObject(*sceneData->pObjects[i]);
            if (i == 0) firstObject = newObject;
            m_Objects.push_back(newObject);
        }

        FreeScene(sceneData);

        return firstObject;
    }
}

#endif // SCENE_SUPPORT