#include "SceneLoading.h"

#ifdef SCENE_SUPPORT

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "scene/Scene.h"

#include "gfx/GfxBuffers.h"
#include "gfx/GfxTexture.h"

#define CGTF_CALL(X) { cgltf_result result = X; ASSERT(result == cgltf_result_success, "CGTF_CALL_FAIL") }

namespace GP
{
	namespace
	{
		void* GetBufferData(cgltf_accessor* accessor)
		{
			unsigned char* buffer = (unsigned char*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset;
			void* data = buffer + accessor->offset;
			return data;
		}

		GfxIndexBuffer* GetIndices(cgltf_accessor* indexAccessor)
		{
			ASSERT(indexAccessor, "[SceneLoading] Trying to read indices from empty accessor");
			ASSERT(indexAccessor->type == cgltf_type_scalar, "[SceneLoading] Indices of a mesh arent scalar.");

			unsigned short* indexData = (unsigned short*)GetBufferData(indexAccessor);
			GfxIndexBuffer* indexBuffer = new GfxIndexBuffer(indexData, indexAccessor->count, cgltf_component_size(indexAccessor->component_type));
			indexBuffer->GetBufferResource()->Initialize();

			return indexBuffer;
		}

		template<typename T>
		GfxVertexBuffer<T>* GetEmptyVB(unsigned int numVertices)
		{
			void* vbData = calloc(numVertices, sizeof(T));
			GfxVertexBuffer<T>* vb = new GfxVertexBuffer<T>(vbData, numVertices);
			vb->GetBufferResource()->Initialize();
			free(vbData);
			return vb;
		}

		template<typename T, cgltf_type TYPE, cgltf_component_type COMPONENT_TYPE>
		GfxVertexBuffer<T>* GetVB(cgltf_attribute* vertexAttribute)
		{
			ASSERT(vertexAttribute->data->type == TYPE, "[SceneLoading] ASSERT FAILED: attributeAccessor->type == TYPE");
			ASSERT(vertexAttribute->data->component_type == COMPONENT_TYPE, "[SceneLoading] ASSERT FAILED: attributeAccessor->component_type == COMPONENT_TYPE");

			T* attributeData = (T*)GetBufferData(vertexAttribute->data);
			GfxVertexBuffer<T>* vertexBuffer = new GfxVertexBuffer<T>(attributeData, vertexAttribute->data->count);
			vertexBuffer->GetBufferResource()->Initialize();

			return vertexBuffer;
		}
	}

	void SceneLoadingJob::LoadScene()
	{
		if (m_PreviousJob)
		{
			m_PreviousJob->WaitToLoad();
			delete m_PreviousJob;
		}

		LOG("Loading scene");

		cgltf_options options = {};
		cgltf_data* data = NULL;
		CGTF_CALL(cgltf_parse_file(&options, m_Path.c_str(), &data));
		CGTF_CALL(cgltf_load_buffers(&options, data, m_Path.c_str()));

		std::vector<SceneObject*> sceneObjects;
		for (size_t i = 0; i < data->meshes_count; i++)
		{
			cgltf_mesh* meshData = (data->meshes + i);
			for (size_t j = 0; j < meshData->primitives_count; j++)
			{
				if (!m_Running) break; // Something requested stop

				SceneObject* sceneObject = LoadSceneObject(meshData->primitives + j);
				sceneObject->SetPostition(m_ScenePosition);
				sceneObject->SetScale(m_SceneScale);
				sceneObject->SetRotation(m_SceneRotation);
				sceneObjects.push_back(sceneObject);

				if (sceneObjects.size() >= BATCH_SIZE)
				{
					m_Scene->AddSceneObjects(sceneObjects);
					sceneObjects.clear();
				}
				
			}
		}
		m_Scene->AddSceneObjects(sceneObjects);

		cgltf_free(data);
	}

	SceneObject* SceneLoadingJob::LoadSceneObject(cgltf_primitive* meshData)
	{
		Mesh* mesh = LoadMesh(meshData);
		Material* material = LoadMaterial(meshData->material);

		return new SceneObject{ mesh, material };
	}

	Mesh* SceneLoadingJob::LoadMesh(cgltf_primitive* meshData)
	{
		ASSERT(meshData->type == cgltf_primitive_type_triangles, "[SceneLoading] Scene contains quad meshes. We are supporting just triangle meshes.");

		GfxVertexBuffer<Vec3>* positionBuffer = nullptr;
		GfxVertexBuffer<Vec2>* uvBuffer = nullptr;
		GfxVertexBuffer<Vec3>* normalBuffer = nullptr;
		GfxVertexBuffer<Vec4>* tangentBuffer = nullptr;

		for (size_t i = 0; i < meshData->attributes_count; i++)
		{
			cgltf_attribute* vertexAttribute = (meshData->attributes + i);
			switch (vertexAttribute->type)
			{
			case cgltf_attribute_type_position:
				positionBuffer = GetVB<Vec3, cgltf_type_vec3, cgltf_component_type_r_32f>(vertexAttribute);
				break;
			case cgltf_attribute_type_texcoord:
				uvBuffer = GetVB<Vec2, cgltf_type_vec2, cgltf_component_type_r_32f>(vertexAttribute);
				break;
			case cgltf_attribute_type_normal:
				normalBuffer = GetVB<Vec3, cgltf_type_vec3, cgltf_component_type_r_32f>(vertexAttribute);
				break;
			case cgltf_attribute_type_tangent:
				tangentBuffer = GetVB<Vec4, cgltf_type_vec4, cgltf_component_type_r_32f>(vertexAttribute);
				break;
			}
		}

		unsigned int vertCount = meshData->attributes->data->count;
		if (!positionBuffer) positionBuffer = GetEmptyVB<Vec3>(vertCount);
		if (!uvBuffer) uvBuffer = GetEmptyVB<Vec2>(vertCount);
		if (!normalBuffer) normalBuffer = GetEmptyVB<Vec3>(vertCount);
		if (!tangentBuffer) tangentBuffer = GetEmptyVB<Vec4>(vertCount);

		GfxIndexBuffer* indexBuffer = GetIndices(meshData->indices);

		return new Mesh{ positionBuffer, uvBuffer, normalBuffer, tangentBuffer, indexBuffer };
	}

	Material* SceneLoadingJob::LoadMaterial(cgltf_material* materialData)
	{
		ASSERT(materialData->has_pbr_metallic_roughness, "[SceneLoading] Every material must have a base color texture!");
		GfxTexture2D* diffuseTexture = nullptr;
		const bool  isTransparent = materialData->alpha_mode == cgltf_alpha_mode_blend;
		if (materialData->pbr_metallic_roughness.base_color_texture.texture)
		{
			std::string imageURI = materialData->pbr_metallic_roughness.base_color_texture.texture->image->uri;
			std::string diffuseTexturePath = m_FolderPath + "/" + imageURI;
			diffuseTexture = new GfxTexture2D(diffuseTexturePath, MAX_MIPS);
		}
		else
		{
			cgltf_float* diffuseColorFloat = materialData->pbr_metallic_roughness.base_color_factor;
			unsigned char diffuseColor[] = { 255 * diffuseColorFloat[0], 255 * diffuseColorFloat[1] , 255 * diffuseColorFloat[2] , 255 * diffuseColorFloat[3] };
			diffuseTexture = new GfxTexture2D(1, 1);
			diffuseTexture->Upload(diffuseColor);
		}

		return new Material{ isTransparent, diffuseTexture };
	}
}

#endif //SCENE_SUPPORT