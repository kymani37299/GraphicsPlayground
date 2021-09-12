#include "gfx/device/DxTexture.h"

#include "core/Core.h"
#include "gfx/device/DxCommon.h"
#include "gfx/device/DxDevice.h"

DXGI_FORMAT ToDXGIFormat(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::RGBA8_UNORM:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case TextureFormat::RGBA_FLOAT:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    default:
        NOT_IMPLEMENTED;
    }

    return DXGI_FORMAT_UNKNOWN;
}

unsigned int ToBPP(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::RGBA8_UNORM:
        return 4;
    case TextureFormat::RGBA_FLOAT:
        return 16;
    default:
        NOT_IMPLEMENTED;
    }

    return 0;
}

DxTexture::DxTexture(DxDevice* device, const TextureDesc& desc):
    m_Width(desc.width),
    m_Height(desc.height)
{
    const unsigned int arraySize = desc.type == TextureType::Cubemap ? 6 : 1;
    ASSERT(arraySize == desc.texData.size(), "[DxTexture] arraySize == desc.texData.size()");

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = desc.width;
    textureDesc.Height = desc.height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = arraySize;
    textureDesc.Format = ToDXGIFormat(desc.format);
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.MiscFlags = desc.type == TextureType::Cubemap ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

    D3D11_SUBRESOURCE_DATA* textureSubresourceData = new D3D11_SUBRESOURCE_DATA[arraySize];
    for (size_t i = 0; i < arraySize; i++)
    {
        textureSubresourceData[i].pSysMem = (const void*) desc.texData[i];
        textureSubresourceData[i].SysMemPitch = ToBPP(desc.format) * desc.width;
    }

    DX_CALL(device->GetDevice()->CreateTexture2D(&textureDesc, textureSubresourceData, &m_Texture));
    DX_CALL(device->GetDevice()->CreateShaderResourceView(m_Texture, nullptr, &m_TextureView));
}

DxTexture::~DxTexture()
{
    m_Texture->Release();
    m_TextureView->Release();
}