#pragma once

#include <vector>
#include <string>

struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

class DxDevice;
class DxConverter;

enum class TextureType
{
	Texture2D,
	Cubemap
};

enum class TextureFormat
{
	RGBA8_UNORM,
	RGBA_FLOAT
};

struct TextureDesc
{
	std::vector<const void*> texData;
	int width;
	int height;
	TextureType type = TextureType::Texture2D;
	TextureFormat format = TextureFormat::RGBA8_UNORM;
};

class DxTexture
{
	friend class DxConverter;

private:
	DxTexture() {}

public:
	DxTexture(DxDevice* device, const TextureDesc& desc);
	~DxTexture();

	inline unsigned int GetWidth() const { return m_Width; }
	inline unsigned int GetHeight() const { return m_Height; }

	inline ID3D11ShaderResourceView* GetTextureView() const { return m_TextureView; }

private:
	unsigned int m_Width;
	unsigned int m_Height;

	ID3D11Texture2D* m_Texture;
	ID3D11ShaderResourceView* m_TextureView;
};