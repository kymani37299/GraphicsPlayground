#include "DxConverter.h"

#include "gfx/device/DxCommon.h"

#include "gfx/device/DxDevice.h"
#include "gfx/device/DxTexture.h"
#include "gfx/device/DxRenderTarget.h"

DxTexture* DxConverter::ToDxTexture(DxDevice* device, DxCubemapRenderTarget*& rt)
{
	DxTexture* texture = new DxTexture();
	texture->m_Height = rt->GetHeight();
	texture->m_Width = rt->GetWidth();
	texture->m_Texture = rt->m_TextureMap;
	DX_CALL(device->GetDevice()->CreateShaderResourceView(rt->m_TextureMap, nullptr, &texture->m_TextureView));

	rt->m_TextureMap = nullptr;
	delete rt;
	rt = nullptr;

	return texture;
}