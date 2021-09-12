#pragma once

#include <string>

#include "Common.h"

namespace GP
{

	namespace ModelLoading
	{
		struct MaterialData;
	}

	class GfxTexture;
	template<typename T>
	class GfxConstantBuffer;
	class GfxDevice;

	struct CBMaterial
	{
		alignas(16) Vec3 diffuse;
		int hasDiffuseMap;
		float metallic;
		int hasMetallicMap;
		float roughness;
		int hasRoughnessMap;
		float ao;
		int hasAoMap;
	};

	class Material
	{

	public:
		Material(const ModelLoading::MaterialData& data);
		~Material();

		inline GfxConstantBuffer<CBMaterial>* GetBuffer() const { return m_Buffer; }
		inline GfxTexture* GetDiffuseTexture() const { return m_UseDiffuseTexture ? m_DiffuseTexture : nullptr; }
		inline GfxTexture* GetMetallicTexture() const { return m_UseMetallicTexture ? m_MetallicTexture : nullptr; }
		inline GfxTexture* GetRoughnessTexture() const { return m_UseRoughnessTexture ? m_RoughnessTexture : nullptr; }
		inline GfxTexture* GetAoTexture() const { return m_UseAoTexture ? m_AoTexture : nullptr; }
		inline bool UseDiffuseTexture() const { return m_UseDiffuseTexture; }

		void SetDiffuseColor(Vec3 diffuseColor);

	private:
		void UpdateBuffer();

	private:
		GfxConstantBuffer<CBMaterial>* m_Buffer;

		GfxTexture* m_DiffuseTexture = nullptr;
		Vec3 m_DiffuseColor = VEC3_ZERO;
		bool m_UseDiffuseTexture = false;

		GfxTexture* m_MetallicTexture = nullptr;
		float m_Metallic = 0.5f;
		bool m_UseMetallicTexture = false;

		GfxTexture* m_RoughnessTexture = nullptr;
		float m_Roughness = 0.5f;
		bool m_UseRoughnessTexture = false;

		GfxTexture* m_AoTexture = nullptr;
		float m_Ao = 0.05f;
		bool m_UseAoTexture = false;
	};

}