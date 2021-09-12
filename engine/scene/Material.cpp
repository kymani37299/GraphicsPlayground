#include "Material.h"

#include <core/GfxCore.h>

#include "util/ModelLoading.h"

namespace GP
{

	GfxTexture* LoadMaterialTexture(ModelLoading::TextureData* textureData, bool& useTex)
	{
		useTex = textureData != nullptr;

		if (!useTex) return nullptr;

		std::vector<const void*> texData;
		texData.push_back(textureData->pData);

		TextureDesc texDesc = {};
		texDesc.texData = texData;
		texDesc.width = textureData->width;
		texDesc.height = textureData->height;

		return new GfxTexture(texDesc);
	}

	Material::Material(const ModelLoading::MaterialData& data) :
		m_DiffuseColor(data.diffuse)
	{
		m_Buffer = new GfxConstantBuffer<CBMaterial>();

		m_DiffuseColor = data.diffuse;
		m_DiffuseTexture = LoadMaterialTexture(data.diffuseMap, m_UseDiffuseTexture);
		m_MetallicTexture = LoadMaterialTexture(data.metallicMap, m_UseMetallicTexture);
		m_RoughnessTexture = LoadMaterialTexture(data.roughnessMap, m_UseRoughnessTexture);
		m_AoTexture = LoadMaterialTexture(data.aoMap, m_UseAoTexture);

		UpdateBuffer();
	}

	Material::~Material()
	{
		delete m_Buffer;
		SAFE_DELETE(m_DiffuseTexture);
	}

	void Material::SetDiffuseColor(Vec3 diffuseColor)
	{
		m_DiffuseColor = diffuseColor;
		UpdateBuffer();
	}

	void Material::UpdateBuffer()
	{
		CBMaterial cbMaterial;
		cbMaterial.diffuse = m_DiffuseColor;
		cbMaterial.hasDiffuseMap = m_UseDiffuseTexture;
		cbMaterial.metallic = m_Metallic;
		cbMaterial.hasMetallicMap = m_UseMetallicTexture;
		cbMaterial.roughness = m_Roughness;
		cbMaterial.hasRoughnessMap = m_UseRoughnessTexture;
		cbMaterial.ao = m_Ao;
		cbMaterial.hasAoMap = m_UseAoTexture;
		m_Buffer->Upload(cbMaterial);
	}

}