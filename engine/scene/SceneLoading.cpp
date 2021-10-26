#include "SceneLoading.h"

#ifdef SCENE_SUPPORT

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "scene/Scene.h"
#include "util/PathUtil.h"

#include "gfx/GfxBuffers.h"
#include "gfx/GfxTexture.h"

#define CGTF_CALL(X) ASSERT(X == cgltf_result_success, "CGTF_CALL_FAIL")

namespace GP
{
	namespace SceneLoading
	{
		static std::string g_Path;

		void* GetBufferData(cgltf_accessor* accessor)
		{
			unsigned char* buffer = (unsigned char*) accessor->buffer_view->buffer->data + accessor->buffer_view->offset;
			void* data = buffer + accessor->offset;
			return data;
		}

		GfxIndexBuffer* GetIndices(cgltf_accessor* indexAccessor)
		{
			ASSERT(indexAccessor->component_type == cgltf_component_type_r_16u &&
				indexAccessor->type == cgltf_type_scalar, "[SceneLoading] Indices of a mesh arent U16.");
			ASSERT(indexAccessor->stride == sizeof(unsigned short), "[SceneLoading] ASSERT FAILED: indexAccessor->stride == sizeof(unsigned short)");

			unsigned short* indexData = (unsigned short*)GetBufferData(indexAccessor);
			GfxIndexBuffer* indexBuffer = new GfxIndexBuffer(indexData, indexAccessor->count);
			indexBuffer->GetBufferResource()->Initialize();

			return indexBuffer;
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

		Mesh* LoadMesh(cgltf_primitive* meshData)
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

			if (!tangentBuffer)
			{
				unsigned vertCount = meshData->attributes->data->count;
				void* tangentData = calloc(vertCount, sizeof(Vec4));
				tangentBuffer = new GfxVertexBuffer<Vec4>(tangentData, vertCount);
				tangentBuffer->GetBufferResource()->Initialize();
				free(tangentData);
			}

			ASSERT(positionBuffer && uvBuffer && normalBuffer && tangentBuffer, "[SceneLoading] Invalid vertex data!");

			GfxIndexBuffer* indexBuffer = GetIndices(meshData->indices);

			return new Mesh{positionBuffer, uvBuffer, normalBuffer, tangentBuffer, indexBuffer};
		}
		
		Material* LoadMaterial(cgltf_material* materialData)
		{
			std::string imageURI = materialData->pbr_metallic_roughness.base_color_texture.texture->image->uri;
			std::string diffuseTexturePath = g_Path + "/" + imageURI;
			GfxTexture2D* diffuseTexture = new GfxTexture2D(diffuseTexturePath);

			return new Material{ diffuseTexture };
		}

		SceneObject* LoadSceneObject(cgltf_primitive* meshData)
		{
			Mesh* mesh = LoadMesh(meshData);
			Material* material = LoadMaterial(meshData->material);

			return new SceneObject{ mesh, material };
		}

		Scene* LoadScene(const std::string& path)
		{
			g_Path = path;

			const std::string purePath = PathUtil::GetPathWitoutFile(path);
			const std::string& ext = PathUtil::GetFileExtension(path);
			ASSERT(ext == "gltf", "[SceneLoading] For now we only support giTF 3D format.");

			cgltf_options options = {};
			cgltf_data* data = NULL;
			CGTF_CALL(cgltf_parse_file(&options, path.c_str(), &data));
			CGTF_CALL(cgltf_load_buffers(&options, data, path.c_str()));

			std::vector<SceneObject*> sceneObjects;
			for (size_t i = 0; i < data->meshes_count; i++)
			{
				cgltf_mesh* meshData = (data->meshes + i);
				for (size_t j = 0; j < meshData->primitives_count; j++)
				{
					sceneObjects.push_back(LoadSceneObject(meshData->primitives + j));
				}
			}

			cgltf_free(data);

			return new Scene{ sceneObjects };
		}
	}
}

#endif //SCENE_SUPPORT