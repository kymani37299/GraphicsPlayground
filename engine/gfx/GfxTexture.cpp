#include "GfxTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <d3d11_1.h>

#include "gfx/GfxCore.h"

namespace GP
{
    namespace
    {
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

    }

    unsigned char INVALID_TEXTURE_COLOR[] = { 0xff, 0x00, 0x33, 0xff };

    void* LoadTexture(const std::string& path, int& width, int& height, int& bpp)
    {
        void* data = stbi_load(path.c_str(), &width, &height, &bpp, 4);

        if (!data)
        {
            LOG("Failed to load texture: " + path);
            data = INVALID_TEXTURE_COLOR;
            width = 1;
            height = 1;
            bpp = 4;
        }

        return data;
    }

    void FreeTexture(void* data)
    {
        if (data != INVALID_TEXTURE_COLOR)
            stbi_image_free(data);
    }

    ///////////////////////////////////////////
    /// Texture                          /////
    /////////////////////////////////////////

    GfxTexture2D::GfxTexture2D(const std::string& path, unsigned int numMips):
        m_Format(TextureFormat::RGBA8_UNORM),
        m_NumMips(numMips)
    {
        // TODO: Generate mips
    
        int width, height, bpp;
        void* texData = LoadTexture(path, width, height, bpp);
        ASSERT(texData, "[GfxTexture2D] Failed to load a texture data.");
        
        // TODO: Fix this, some texutres have bpp = 3 for some reason.
        //ASSERT(bpp == 4, "[GfxTexture2D] Failed loading texture. We are only supporting RGBA8 textures.");
        
        m_Width = width;
        m_Height = height;

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = ToDXGIFormat(m_Format);
        textureDesc.SampleDesc.Count = 1; // TODO: Support MSAA
        textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;
    
        D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
        textureSubresourceData.pSysMem = texData;
        textureSubresourceData.SysMemPitch = m_Width * ToBPP(m_Format);
    
        // TODO:
        //D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        //srvDesc.Format = ToDXGIFormat(m_Format);
        //srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        //srvDesc.Texture2D.MipLevels = m_NumMips;
        //srvDesc.Texture2D.MostDetailedMip = 0;
    
        DX_CALL(g_Device->GetDevice()->CreateTexture2D(&textureDesc, &textureSubresourceData, &m_Texture));
        DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(m_Texture, nullptr, &m_TextureView));
    
        FreeTexture(texData);
    }

    GfxTexture2D::~GfxTexture2D()
    {
        m_Texture->Release();
        m_TextureView->Release();
    }

    ///////////////////////////////////////////
    /// Texture                          /////
    /////////////////////////////////////////

    GfxTexture::GfxTexture(const TextureDesc& desc) :
        m_Width(desc.width),
        m_Height(desc.height)
    {
        const unsigned int arraySize = desc.type == TextureType::Cubemap ? 6 : 1;
        ASSERT(arraySize == desc.texData.size(), "[GfxTexture] arraySize == desc.texData.size()");

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
            textureSubresourceData[i].pSysMem = (const void*)desc.texData[i];
            textureSubresourceData[i].SysMemPitch = ToBPP(desc.format) * desc.width;
        }

        DX_CALL(g_Device->GetDevice()->CreateTexture2D(&textureDesc, textureSubresourceData, &m_Texture));
        DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(m_Texture, nullptr, &m_TextureView));
    }

    GfxTexture::~GfxTexture()
    {
        m_Texture->Release();
        m_TextureView->Release();
    }

    ///////////////////////////////////////////
    /// RenderTarget                     /////
    /////////////////////////////////////////

    GfxRenderTarget* GfxRenderTarget::CreateFromSwapChain(IDXGISwapChain1* swapchain)
    {
        GfxRenderTarget* rt = new GfxRenderTarget();
        rt->m_NumRTs = 1;
        rt->m_RTViews.resize(1);
        rt->m_SRViews.resize(1);
        rt->m_SRViews[0] = nullptr;
        rt->m_DSSRView = nullptr;
        rt->m_Width = WINDOW_WIDTH;
        rt->m_Height = WINDOW_HEIGHT;

        ID3D11Device1* d = g_Device->GetDevice();

        ID3D11Texture2D* d3d11FrameBuffer;
        DX_CALL(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer));
        DX_CALL(d->CreateRenderTargetView(d3d11FrameBuffer, 0, &rt->m_RTViews[0]));

        // Depth stencil
        D3D11_TEXTURE2D_DESC dsDesc = {};
        dsDesc.Width = WINDOW_WIDTH;
        dsDesc.Height = WINDOW_HEIGHT;
        dsDesc.MipLevels = 1;
        dsDesc.ArraySize = 1;
        dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsDesc.SampleDesc.Count = 1;
        dsDesc.SampleDesc.Quality = 0;
        dsDesc.Usage = D3D11_USAGE_DEFAULT;
        dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        dsDesc.CPUAccessFlags = 0;
        dsDesc.MiscFlags = 0;

        ID3D11Texture2D* dsTextureMap;
        DX_CALL(d->CreateTexture2D(&dsDesc, NULL, &dsTextureMap));

        D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
        dsViewDesc.Format = dsDesc.Format;
        dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsViewDesc.Texture2D.MipSlice = 0;

        DX_CALL(d->CreateDepthStencilView(dsTextureMap, &dsViewDesc, &rt->m_DSView));

        return rt;
    }

    GfxRenderTarget::GfxRenderTarget(const RenderTargetDesc& desc)
    {
        m_NumRTs = desc.numRTs;
        m_Width = desc.width;
        m_Height = desc.height;

        CreateRenderTargets(g_Device->GetDevice(), desc);

        if (desc.useDepth)
        {
            CreateDepthStencil(g_Device->GetDevice(), desc);
        }
    }

    GfxRenderTarget::~GfxRenderTarget()
    {
        SAFE_RELEASE(m_TextureMap);

        for (unsigned int i = 0; i < m_NumRTs; i++)
        {
            m_RTViews[i]->Release();
            SAFE_RELEASE(m_SRViews[i]);
        }

        if (m_DSView)
        {
            m_DSView->Release();
            SAFE_RELEASE(m_DSSRView);
        }
    }

    void GfxRenderTarget::CreateRenderTargets(ID3D11Device1* device, const RenderTargetDesc& desc)
    {
        m_RTViews.resize(m_NumRTs);
        m_SRViews.resize(m_NumRTs);

        for (size_t i = 0; i < m_NumRTs; i++)
        {
            D3D11_TEXTURE2D_DESC textureDesc = {};
            textureDesc.Width = desc.width;
            textureDesc.Height = desc.height;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;

            DX_CALL(device->CreateTexture2D(&textureDesc, NULL, &m_TextureMap));

            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = textureDesc.Format;
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            renderTargetViewDesc.Texture2D.MipSlice = 0;

            DX_CALL(device->CreateRenderTargetView(m_TextureMap, &renderTargetViewDesc, &m_RTViews[i]));

            D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
            shaderResourceViewDesc.Format = textureDesc.Format;
            shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
            shaderResourceViewDesc.Texture2D.MipLevels = 1;

            DX_CALL(device->CreateShaderResourceView(m_TextureMap, &shaderResourceViewDesc, &m_SRViews[i]));
        }
    }

    void GfxRenderTarget::CreateDepthStencil(ID3D11Device1* device, const RenderTargetDesc& desc)
    {
        ASSERT(desc.useDepth, "[GfxRenderTarget::CreateDepthStencil] desc.useDepth == false");

        const DXGI_FORMAT DEPTH_FORMAT = desc.useStencil ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_R32_TYPELESS;
        const DXGI_FORMAT DSV_FORMAT = desc.useStencil ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT;
        const DXGI_FORMAT DSRV_FORMAT = desc.useStencil ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_R32_FLOAT;
        const DXGI_FORMAT SSRV_FORMAT = DXGI_FORMAT_X24_TYPELESS_G8_UINT;

        D3D11_TEXTURE2D_DESC dsDesc = {};
        dsDesc.Width = desc.width;
        dsDesc.Height = desc.height;
        dsDesc.MipLevels = 1;
        dsDesc.ArraySize = 1;
        dsDesc.Format = DEPTH_FORMAT;
        dsDesc.SampleDesc.Count = 1;
        dsDesc.SampleDesc.Quality = 0;
        dsDesc.Usage = D3D11_USAGE_DEFAULT;
        dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        dsDesc.CPUAccessFlags = 0;
        dsDesc.MiscFlags = 0;

        ID3D11Texture2D* dsTextureMap;
        DX_CALL(device->CreateTexture2D(&dsDesc, NULL, &dsTextureMap));

        D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
        dsViewDesc.Format = DSV_FORMAT;
        dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsViewDesc.Texture2D.MipSlice = 0;

        DX_CALL(device->CreateDepthStencilView(dsTextureMap, &dsViewDesc, &m_DSView));

        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
        shaderResourceViewDesc.Format = DSRV_FORMAT;
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
        shaderResourceViewDesc.Texture2D.MipLevels = 1;

        DX_CALL(device->CreateShaderResourceView(dsTextureMap, &shaderResourceViewDesc, &m_DSSRView));
    }

    ///////////////////////////////////////////
    /// CubemapRenderTarget            /////
    /////////////////////////////////////////

    GfxCubemapRenderTarget::GfxCubemapRenderTarget(const RenderTargetDesc& desc) :
        m_Width(desc.width),
        m_Height(desc.height)
    {
        ID3D11Device1* d = g_Device->GetDevice();

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = desc.width;
        textureDesc.Height = desc.height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 6;
        textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        DX_CALL(d->CreateTexture2D(&textureDesc, NULL, &m_TextureMap));

        m_RTViews.resize(6);
        for (size_t i = 0; i < 6; i++)
        {
            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = textureDesc.Format;
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            renderTargetViewDesc.Texture2DArray.MipSlice = 0;
            renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
            renderTargetViewDesc.Texture2DArray.ArraySize = 1;

            DX_CALL(d->CreateRenderTargetView(m_TextureMap, &renderTargetViewDesc, &m_RTViews[i]));
        }

        DX_CALL(d->CreateShaderResourceView(m_TextureMap, nullptr, &m_SRView));
    }

    GfxCubemapRenderTarget::~GfxCubemapRenderTarget()
    {
        SAFE_RELEASE(m_TextureMap);
        m_SRView->Release();
        for (size_t i = 0; i < 6; i++) m_RTViews[i]->Release();
    }
}