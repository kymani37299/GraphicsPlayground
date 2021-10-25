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

    TextureResource2D::~TextureResource2D()
    {
        SAFE_RELEASE(m_Resource);
    }

    ///////////////////////////////////////////
    /// GfxTexture2D                     /////
    /////////////////////////////////////////

    GfxTexture2D::GfxTexture2D(const std::string& path, unsigned int numMips)
    {
        const TextureFormat texFormat = TextureFormat::RGBA8_UNORM;

        // TODO: Generate mips
    
        int width, height, bpp;
        void* texData = LoadTexture(path, width, height, bpp);
        ASSERT(texData, "[GfxTexture2D] Failed to load a texture data.");
        
        // TODO: Fix this, some texutres have bpp = 3 for some reason.
        //ASSERT(bpp == 4, "[GfxTexture2D] Failed loading texture. We are only supporting RGBA8 textures.");

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1; // TODO: Support mip generation
        textureDesc.ArraySize = 1;
        textureDesc.Format = ToDXGIFormat(texFormat);
        textureDesc.SampleDesc.Count = 1; // TODO: Support MSAA
        textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;
    
        D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
        textureSubresourceData.pSysMem = texData;
        textureSubresourceData.SysMemPitch = width * ToBPP(texFormat);
    
        // TODO:
        //D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        //srvDesc.Format = ToDXGIFormat(m_Format);
        //srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        //srvDesc.Texture2D.MipLevels = m_NumMips;
        //srvDesc.Texture2D.MostDetailedMip = 0;

        ID3D11Texture2D* tex2D;
        DX_CALL(g_Device->GetDevice()->CreateTexture2D(&textureDesc, &textureSubresourceData, &tex2D));
        DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(tex2D, nullptr, &m_SRV));
        
        m_Resource = new TextureResource2D(tex2D, width, height, texFormat, numMips, 1);

        FreeTexture(texData);
    }

    GfxTexture2D::GfxTexture2D(TextureResource2D* textureResource, unsigned int arrayIndex):
        m_Resource(textureResource)
    {
        textureResource->AddRef();

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = ToDXGIFormat(textureResource->GetFormat());
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.FirstArraySlice = arrayIndex;
        srvDesc.Texture2DArray.ArraySize = 1;
        srvDesc.Texture2DArray.MipLevels = 1; // TODO: MipMaps
        srvDesc.Texture2DArray.MostDetailedMip = 0;

        DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(textureResource->GetResource(), &srvDesc, &m_SRV));
    }

    GfxTexture2D::~GfxTexture2D()
    {
        m_Resource->Release();
        m_SRV->Release();
    }

    ///////////////////////////////////////////
    /// GfxCubemap                       /////
    /////////////////////////////////////////

    // The order of textures:  Right, Left, Up, Down, Back, Front
    GfxCubemap::GfxCubemap(std::string textures[6], unsigned int numMips)
    {
        const TextureFormat texFormat = TextureFormat::RGBA8_UNORM;

        D3D11_SUBRESOURCE_DATA* textureSubresourceData = new D3D11_SUBRESOURCE_DATA[6];
        int texWidth, texHeight;
        void* texData[6];
        for (size_t i = 0; i < 6; i++)
        {
            int width, height, bpp;
            texData[i] = LoadTexture(textures[i], width, height, bpp);
            if (i == 0) // Fill the width and height data from first face
            {
                texWidth = width;
                texHeight = height;
            }
            ASSERT(texData[i], "[GfxCubemap] Error loading face data: " + textures[i]);
            //ASSERT(bpp == 4, "[GfxCubemap] Failed loading face data. We are only supporting RGBA8 textures.") TODO: Fix this, some texutres have bpp = 3 for some reason.
            ASSERT(texWidth == width && texHeight == height, "[GfxCubemap] Error: Face data size doesn't match with other faces : " + textures[i]);

            textureSubresourceData[i].pSysMem = texData[i];
            textureSubresourceData[i].SysMemPitch = texWidth * ToBPP(texFormat);
            textureSubresourceData[i].SysMemSlicePitch = 0;
        }

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = texWidth;
        textureDesc.Height = texHeight;
        textureDesc.MipLevels = 1; // TODO: Support mip generation
        textureDesc.ArraySize = 6;
        textureDesc.Format = ToDXGIFormat(texFormat);
        textureDesc.SampleDesc.Count = 1; // TODO: Support MSAA
        textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        ID3D11Texture2D* tex2D;
        DX_CALL(g_Device->GetDevice()->CreateTexture2D(&textureDesc, textureSubresourceData, &tex2D));
        DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(tex2D, nullptr, &m_SRV));

        m_Resource = new TextureResource2D(tex2D, texWidth, texHeight, texFormat, numMips, 6);

        // Free texture memory
        for (size_t i = 0; i < 6; i++)
        {
            FreeTexture(texData[i]);
        }
    }

    GfxCubemap::GfxCubemap(TextureResource2D* textureResource):
        m_Resource(textureResource)
    {
        ASSERT(textureResource->GetArraySize() == 6, "[GfxCubemap] Trying to create cubemap with resource that doesn't have arraySize=6");

        textureResource->AddRef();
        DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(textureResource->GetResource(), nullptr, &m_SRV));
    }

    GfxCubemap::~GfxCubemap()
    {
        m_SRV->Release();
        m_Resource->Release();
    }

    ///////////////////////////////////////////
    /// RenderTarget                     /////
    /////////////////////////////////////////

    GfxRenderTarget* GfxRenderTarget::CreateFromSwapChain(IDXGISwapChain1* swapchain)
    {
        GfxRenderTarget* rt = new GfxRenderTarget();
        rt->m_CreationFlags = 0;
        rt->m_NumRTs = 1;
        rt->m_RTVs.resize(1);
        rt->m_Resources.resize(1);

        ID3D11Device1* d = g_Device->GetDevice();

        ID3D11Texture2D* d3d11FrameBuffer;
        DX_CALL(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer));
        rt->m_Resources[0] = new TextureResource2D(d3d11FrameBuffer, WINDOW_WIDTH, WINDOW_HEIGHT, TextureFormat::UNKNOWN, 1, 1);

        DX_CALL(d->CreateRenderTargetView(d3d11FrameBuffer, 0, &rt->m_RTVs[0]));

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
        rt->m_DepthResource = new TextureResource2D(dsTextureMap, WINDOW_WIDTH, WINDOW_HEIGHT, TextureFormat::UNKNOWN, 1, 1);

        D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
        dsViewDesc.Format = dsDesc.Format;
        dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsViewDesc.Texture2D.MipSlice = 0;

        DX_CALL(d->CreateDepthStencilView(dsTextureMap, &dsViewDesc, &rt->m_DSV));

        return rt;
    }

    GfxRenderTarget::GfxRenderTarget(unsigned int width, unsigned int height, unsigned int numRTs, unsigned int creationFlags):
        m_NumRTs(numRTs),
        m_CreationFlags(creationFlags)
    {
        m_RTVs.resize(numRTs);
        m_Resources.resize(numRTs);

        const TextureFormat texFormat = TextureFormat::RGBA_FLOAT;
        ID3D11Device1* device = g_Device->GetDevice();

        for (size_t i = 0; i < numRTs; i++)
        {
            ID3D11Texture2D* textureMap;

            D3D11_TEXTURE2D_DESC textureDesc = {};
            textureDesc.Width = width;
            textureDesc.Height = height;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = ToDXGIFormat(texFormat);
            textureDesc.SampleDesc.Count = 1; // TODO: MSAA
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
            if (creationFlags & RTCF_SRV) textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;

            DX_CALL(device->CreateTexture2D(&textureDesc, NULL, &textureMap));

            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = textureDesc.Format;
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            renderTargetViewDesc.Texture2D.MipSlice = 0;

            DX_CALL(device->CreateRenderTargetView(textureMap, &renderTargetViewDesc, &m_RTVs[i]));

            m_Resources[i] = new TextureResource2D(textureMap, width, height, texFormat, 1, 1);
        }

        if (creationFlags & RTCF_UseDepth || creationFlags & RTCF_UseStencil)
        {
            ASSERT(creationFlags & RTCF_UseDepth, "[GfxRenderTarget] We can use stencil just with depth for now!");

            const bool useStencil = creationFlags & RTCF_UseStencil;
            const DXGI_FORMAT DEPTH_FORMAT = useStencil ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_R32_TYPELESS;
            const DXGI_FORMAT DSV_FORMAT = useStencil ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT;

            // TODO: We need to use this format when making SRV to depth stencil
            //const DXGI_FORMAT DSRV_FORMAT = useStencil ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_R32_FLOAT;
            //const DXGI_FORMAT SSRV_FORMAT = DXGI_FORMAT_X24_TYPELESS_G8_UINT;

            D3D11_TEXTURE2D_DESC dsDesc = {};
            dsDesc.Width = width;
            dsDesc.Height = height;
            dsDesc.MipLevels = 1;
            dsDesc.ArraySize = 1;
            dsDesc.Format = DEPTH_FORMAT;
            dsDesc.SampleDesc.Count = 1;
            dsDesc.SampleDesc.Quality = 0;
            dsDesc.Usage = D3D11_USAGE_DEFAULT;
            dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            if (creationFlags & RTCF_SRV) dsDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            dsDesc.CPUAccessFlags = 0;
            dsDesc.MiscFlags = 0;

            ID3D11Texture2D* dsTextureMap;
            DX_CALL(device->CreateTexture2D(&dsDesc, NULL, &dsTextureMap));
            m_DepthResource = new TextureResource2D(dsTextureMap, width, height, TextureFormat::UNKNOWN, 1, 1);

            D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
            dsViewDesc.Format = DSV_FORMAT;
            dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            dsViewDesc.Texture2D.MipSlice = 0;

            DX_CALL(device->CreateDepthStencilView(dsTextureMap, &dsViewDesc, &m_DSV));
        }
    }

    GfxRenderTarget::~GfxRenderTarget()
    {
        for (unsigned int i = 0; i < m_NumRTs; i++)
        {
            m_RTVs[i]->Release();
            m_Resources[i]->Release();
        }

        SAFE_RELEASE(m_DSV);
        SAFE_RELEASE(m_DepthResource);
    }

    
    ///////////////////////////////////////////
    /// CubemapRenderTarget            /////
    /////////////////////////////////////////

    GfxCubemapRenderTarget::GfxCubemapRenderTarget(unsigned int width, unsigned int height, unsigned int creationFlags):
        m_CreationFlags(creationFlags)
    {
        const TextureFormat texFormat = TextureFormat::RGBA_FLOAT;

        ID3D11Device1* d = g_Device->GetDevice();

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 6;
        textureDesc.Format = ToDXGIFormat(texFormat);
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
        if (creationFlags & RTCF_SRV) textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        ID3D11Texture2D* textureMap;
        DX_CALL(d->CreateTexture2D(&textureDesc, NULL, &textureMap));
        m_Resource = new TextureResource2D(textureMap, width, height, texFormat, 1, 6);

        for (size_t i = 0; i < 6; i++)
        {
            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = textureDesc.Format;
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            renderTargetViewDesc.Texture2DArray.MipSlice = 0;
            renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
            renderTargetViewDesc.Texture2DArray.ArraySize = 1;

            DX_CALL(d->CreateRenderTargetView(textureMap, &renderTargetViewDesc, &m_RTVs[i]));
        }
    }

    GfxCubemapRenderTarget::~GfxCubemapRenderTarget()
    {
        m_Resource->Release();
        for (size_t i = 0; i < 6; i++) m_RTVs[i]->Release();
    }
}