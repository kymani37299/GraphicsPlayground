#pragma once

#include <string>

#include "Common.h"

namespace GP
{
	// STRUCTS
	namespace SceneLoading
	{
		struct TextureData
		{
			const void* pData;
			int width;
			int height;
			int bpp;

			static TextureData* INVALID;
		};

		struct CubemapData
		{
			TextureData* pData[6];
		};

		struct MaterialData
		{
			Vec3 diffuse = { 0.3,0.3,0.3 };
			float metallic = 0.5f;
			float roughness = 0.5f;
			float ao = 0.05f;

			TextureData* diffuseMap = nullptr;
			TextureData* metallicMap = nullptr;
			TextureData* roughnessMap = nullptr;
			TextureData* aoMap = nullptr;
		};

		struct MeshVertex
		{
			Vec3 position;
			Vec3 normal;
			Vec2 uv;

			inline static unsigned int GetStride()
			{
				return (3 + 3 + 2) * sizeof(float);
			}
		};

		struct MeshData
		{
			MeshVertex* pVertices;
			unsigned int numVertices;

			unsigned int* pIndices;
			unsigned int numIndices;
		};

		struct ObjectData
		{
			MeshData* mesh;
			MaterialData material;
		};

		enum class LightType
		{
			Directional,
			Point,
			Spot,
			Ambient,
			Area
		};

		struct LightData
		{
			LightType type;
			Vec3 position;
			Vec3 direction;
			Vec3 attenuation;
			Vec3 color;
		};

		struct SceneData
		{
			ObjectData** pObjects;
			unsigned int numObjects;

			LightData** pLights;
			unsigned int numLights;
		};
	}

	// Functions
	namespace SceneLoading
	{
		ENGINE_DLL SceneData* LoadScene(const char* path);
		ENGINE_DLL void FreeScene(SceneData* scene);

		ENGINE_DLL TextureData* LoadTexture(const std::string& path, bool hdr = false, bool flipY = true);
		ENGINE_DLL void FreeTexture(TextureData* textureData);

		ENGINE_DLL CubemapData* LoadCubemap(const std::string& path);
		ENGINE_DLL void FreeCubemap(CubemapData* cubemap);

		void StartAssimpLogger();
		void StopAssimpLogger();
	}
}
