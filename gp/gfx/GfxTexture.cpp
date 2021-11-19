#include "GfxTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <d3d11_1.h>

#include "gfx/GfxDevice.h"

namespace GP
{
    namespace
    {
        DXGI_FORMAT ToDXGIFormat(TextureFormat format)
        {
            switch (format)
            {
            case TextureFormat::RGBA8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::RGBA_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case TextureFormat::R24G8_TYPELESS: return DXGI_FORMAT_R24G8_TYPELESS;
            case TextureFormat::R32_TYPELESS: return DXGI_FORMAT_R32_TYPELESS;
            case TextureFormat::UNKNOWN: return DXGI_FORMAT_UNKNOWN;
            default: NOT_IMPLEMENTED;
            }

            return DXGI_FORMAT_UNKNOWN;
        }

        DXGI_FORMAT ToDXGIFormatDSV(TextureFormat format)
        {
            switch (format)
            {
            case TextureFormat::R24G8_TYPELESS: return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case TextureFormat::R32_TYPELESS: return DXGI_FORMAT_D32_FLOAT;
            case TextureFormat::UNKNOWN: return DXGI_FORMAT_UNKNOWN;
            default: NOT_IMPLEMENTED;
            }

            return DXGI_FORMAT_UNKNOWN;
        }

        unsigned int ToBPP(TextureFormat format)
        {
            switch (format)
            {
            case TextureFormat::RGBA8_UNORM: return 4;
            case TextureFormat::RGBA_FLOAT: return 16;
            case TextureFormat::R24G8_TYPELESS: return 4;
            case TextureFormat::R32_TYPELESS: return 4;
            case TextureFormat::UNKNOWN: return 0;
            default:
                NOT_IMPLEMENTED;
            }

            return 0;
        }



        D3D11_FILTER GetDXFilter(SamplerFilter filter)
        {
            switch (filter)
            {
            case SamplerFilter::Point: return D3D11_FILTER_MIN_MAG_MIP_POINT;
            case SamplerFilter::Trilinear: case SamplerFilter::Linear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            case SamplerFilter::Anisotropic: return D3D11_FILTER_ANISOTROPIC;
            default: NOT_IMPLEMENTED;
            }
            return D3D11_FILTER_MIN_MAG_MIP_POINT;
        }

        D3D11_TEXTURE_ADDRESS_MODE GetDXMode(SamplerMode mode)
        {
            switch (mode)
            {
            case SamplerMode::Clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
            case SamplerMode::Wrap: return D3D11_TEXTURE_ADDRESS_WRAP;
            case SamplerMode::Border: return D3D11_TEXTURE_ADDRESS_BORDER;
            case SamplerMode::Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
            case SamplerMode::MirrorOnce: return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
            default: NOT_IMPLEMENTED;
            }
            return D3D11_TEXTURE_ADDRESS_WRAP;
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
    }

    ///////////////////////////////////////////
    /// TextureResource2D                /////
    /////////////////////////////////////////

    namespace
    {
        inline D3D11_USAGE GetTextureUsageFlags(unsigned int creationFlags)
        {
            if (creationFlags & TRCF_Static)	return D3D11_USAGE_IMMUTABLE;
            else if (creationFlags & TRCF_Dynamic) return D3D11_USAGE_DYNAMIC;
            else if (creationFlags & TRCF_Staging) return D3D11_USAGE_STAGING;
            return D3D11_USAGE_DEFAULT;
        }

        inline unsigned int GetTextureBindFlags(unsigned int creationFlags)
        {
            unsigned int flags = 0;
            if (creationFlags & TRCF_BindSRV || creationFlags & TRCF_BindCubemap) flags |= D3D11_BIND_SHADER_RESOURCE;
            if (creationFlags & TRCF_BindRT) flags |= D3D11_BIND_RENDER_TARGET;
            if (creationFlags & TRCF_BindDepthStencil) flags |= D3D11_BIND_DEPTH_STENCIL;
            return flags;
        }

        inline unsigned int GetTextureMiscFlags(unsigned int creationFlags)
        {
            unsigned int flags = 0;
            if (creationFlags & TRCF_BindCubemap) flags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
            if (creationFlags & TRCF_GenerateMips) flags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
            return flags;
        }

        inline unsigned int GetTextureCPUAccessFlags(unsigned int creationFlags)
        {
            unsigned int flags = 0;
            if (creationFlags & TRCF_CPURead) flags |= D3D11_CPU_ACCESS_READ;
            if (creationFlags & TRCF_CPUWrite) flags |= D3D11_CPU_ACCESS_WRITE;
            return flags;
        }

        inline D3D11_TEXTURE2D_DESC FillTextureDescription(unsigned int width, unsigned int height, unsigned int numMips, unsigned int arraySize, TextureFormat format, unsigned int creationFlags)
        {
            D3D11_TEXTURE2D_DESC textureDesc = {};
            textureDesc.Width = width;
            textureDesc.Height = height;
            textureDesc.MipLevels = numMips;
            textureDesc.ArraySize = arraySize;
            textureDesc.Format = ToDXGIFormat(format);
            textureDesc.SampleDesc.Count = 1; // TODO: Support MSAA
            textureDesc.Usage = GetTextureUsageFlags(creationFlags);
            textureDesc.BindFlags = GetTextureBindFlags(creationFlags);
            textureDesc.MiscFlags = GetTextureMiscFlags(creationFlags);
            textureDesc.CPUAccessFlags = GetTextureCPUAccessFlags(creationFlags);
            return textureDesc;
        }
    }

    void TextureResource2D::Initialize()
    {
        m_RowPitch = m_Width * ToBPP(m_Format);
        m_SlicePitch = m_RowPitch * m_Height;

        D3D11_TEXTURE2D_DESC textureDesc = FillTextureDescription(m_Width, m_Height, m_NumMips, m_ArraySize, m_Format, m_CreationFlags);
        DX_CALL(g_Device->GetDevice()->CreateTexture2D(&textureDesc, nullptr, &m_Resource));
    }

    void TextureResource2D::InitializeWithData(void* data[])
    {
        //ASSERT(sizeof(data) / sizeof(void*) == m_ArraySize, "[TextureResource2D] Initialization data size doesn't match resource size!");

        m_RowPitch = m_Width * ToBPP(m_Format);
        m_SlicePitch = m_RowPitch * m_Height;

        D3D11_TEXTURE2D_DESC textureDesc = FillTextureDescription(m_Width, m_Height, m_NumMips, m_ArraySize, m_Format, m_CreationFlags);
        D3D11_SUBRESOURCE_DATA* subresourceData = (D3D11_SUBRESOURCE_DATA*) malloc(m_ArraySize * sizeof(D3D11_SUBRESOURCE_DATA));
        for (size_t i = 0; i < m_ArraySize; i++)
        {
            subresourceData[i].pSysMem = data[i];
            subresourceData[i].SysMemPitch = m_RowPitch;
            subresourceData[i].SysMemSlicePitch = m_SlicePitch;
        }

        DX_CALL(g_Device->GetDevice()->CreateTexture2D(&textureDesc, subresourceData, &m_Resource));
    }

    void TextureResource2D::Upload(void* data, unsigned int arrayIndex)
    {
        ASSERT(!(m_CreationFlags & TRCF_Static), "[TextureResource2D] Can't upload to static texture!");

        unsigned int subresourceIndex = D3D11CalcSubresource(0, arrayIndex, m_NumMips);
        g_Device->GetContext()->GetHandle()->UpdateSubresource(m_Resource, subresourceIndex, nullptr, data, m_RowPitch, 0u);
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
        
        int width, height, bpp;
        void* texData[1];
        texData[0] = LoadTexture(path, width, height, bpp);
        ASSERT(texData[0], "[GfxTexture2D] Failed to load a texture data.");
        
        // TODO: Fix this, some texutres have bpp = 3 for some reason.
        //ASSERT(bpp == 4, "[GfxTexture2D] Failed loading texture. We are only supporting RGBA8 textures.");

        
        if (numMips == 1)
        {
            const unsigned int creationFlags = TRCF_BindSRV | TRCF_Static;
            m_Resource = new TextureResource2D(width, height, texFormat, numMips, 1, creationFlags);
            m_Resource->InitializeWithData(texData);
            DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(m_Resource->GetResource(), nullptr, &m_SRV));
        }
        else
        {
            // TODO: Use staging texture for buffer generation

            const unsigned int creationFlags = TRCF_BindSRV | TRCF_BindRT | TRCF_GenerateMips;
            m_Resource = new TextureResource2D(width, height, texFormat, numMips, 1, creationFlags);
            m_Resource->Initialize();
            m_Resource->Upload(texData[0], 0);

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = ToDXGIFormat(m_Resource->GetFormat());
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = numMips == MAX_MIPS ? -1 : numMips;
            srvDesc.Texture2D.MostDetailedMip = 0;

            DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(m_Resource->GetResource(), &srvDesc, &m_SRV));

            g_Device->GetContext()->GetHandle()->GenerateMips(m_SRV);
        }
    
        FreeTexture(texData[0]);

    }

    GfxTexture2D::GfxTexture2D(TextureResource2D* textureResource, unsigned int arrayIndex):
        m_Resource(textureResource)
    {
        // TODO: Check if this even works

        textureResource->AddRef();

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = ToDXGIFormat(textureResource->GetFormat());
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.FirstArraySlice = arrayIndex;
        srvDesc.Texture2DArray.ArraySize = 1;
        srvDesc.Texture2DArray.MipLevels = textureResource->GetNumMips();
        srvDesc.Texture2DArray.MostDetailedMip = 0;

        DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(textureResource->GetResource(), &srvDesc, &m_SRV));
    }

    GfxTexture2D::GfxTexture2D(unsigned int width, unsigned int height, unsigned int numMips)
    {
        const TextureFormat texFormat = TextureFormat::RGBA8_UNORM;

        const unsigned int creationFlags = TRCF_BindSRV;
        m_Resource = new TextureResource2D(width, height, texFormat, numMips, 1, creationFlags);
        m_Resource->Initialize();

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = ToDXGIFormat(m_Resource->GetFormat());
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = numMips == MAX_MIPS ? -1 : numMips;
        srvDesc.Texture2D.MostDetailedMip = 0;

        DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(m_Resource->GetResource(), &srvDesc, &m_SRV));
    }

    GfxTexture2D::~GfxTexture2D()
    {
        m_Resource->Release();
        m_SRV->Release();
    }

    ///////////////////////////////////////////
    /// GfxTextureArray2D                /////
    /////////////////////////////////////////

    GfxTextureArray2D::GfxTextureArray2D(unsigned int width, unsigned int height, unsigned int arraySize, unsigned int numMips)
    {
        const TextureFormat texFormat = TextureFormat::RGBA8_UNORM;

        const unsigned int creationFlags = TRCF_BindSRV;
        m_Resource = new TextureResource2D(width, height, texFormat, numMips, arraySize, creationFlags);
        m_Resource->Initialize();

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = ToDXGIFormat(m_Resource->GetFormat());
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.MipLevels = numMips == MAX_MIPS ? -1 : numMips;
        srvDesc.Texture2DArray.MostDetailedMip = 0;
        srvDesc.Texture2DArray.FirstArraySlice = 0;
        srvDesc.Texture2DArray.ArraySize = arraySize;

        DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(m_Resource->GetResource(), &srvDesc, &m_SRV));
    }

    GfxTextureArray2D::~GfxTextureArray2D()
    {
        m_Resource->Release();
        m_SRV->Release();
    }

    void GfxTextureArray2D::Upload(const std::string& path, unsigned int index)
    {
        int width, height, bpp;
        void* data = LoadTexture(path, width, height, bpp);

        ASSERT(width == m_Resource->GetWidth() && height == m_Resource->GetHeight(), "[GfxTextureArray2D] Trying to upload a texture that has different size than texture array!");
        ASSERT(bpp == ToBPP(m_Resource->GetFormat()), "[GfxTextureArray2D] Trying to upload a texture that has different format than texture array!");

        Upload(index, data);
    }

    ///////////////////////////////////////////
    /// GfxCubemap                       /////
    /////////////////////////////////////////

    // The order of textures:  Right, Left, Up, Down, Back, Front
    GfxCubemap::GfxCubemap(std::string textures[6], unsigned int numMips)
    {
        ASSERT(numMips != MAX_MIPS, "[GfxCubemap] We are currently not supporting MAX_MIPS for Cubemap!");

        const TextureFormat texFormat = TextureFormat::RGBA8_UNORM;
        
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
        }

        if (numMips == 1)
        {
            const unsigned int creationFlags = TRCF_BindCubemap | TRCF_Static;
            m_Resource = new TextureResource2D(texWidth, texHeight, texFormat, numMips, 6, creationFlags);
            m_Resource->InitializeWithData(texData);
            DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(m_Resource->GetResource(), nullptr, &m_SRV));
        }
        else
        {
            // TODO: Use staging texture for mips generation

            const unsigned int creationFlags = TRCF_BindCubemap | TRCF_BindRT | TRCF_GenerateMips;

            m_Resource = new TextureResource2D(texWidth, texHeight, texFormat, numMips, 6, creationFlags);
            m_Resource->Initialize();
            for(size_t i=0;i<6;i++) m_Resource->Upload(texData[i], i);

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = ToDXGIFormat(m_Resource->GetFormat());
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            srvDesc.Texture2DArray.ArraySize = 6;
            srvDesc.Texture2DArray.FirstArraySlice = 0;
            srvDesc.Texture2DArray.MipLevels = numMips == MAX_MIPS ? -1 : numMips;
            srvDesc.Texture2DArray.MostDetailedMip = 0;

            DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(m_Resource->GetResource(), &srvDesc, &m_SRV));

            g_Device->GetContext()->GetHandle()->GenerateMips(m_SRV);
        }

        // Free texture memory
        for (size_t i = 0; i < 6; i++)
        {
            FreeTexture(texData[i]);
        }

        DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(m_Resource->GetResource(), nullptr, &m_SRV));

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
        rt->m_NumRTs = 1;
        rt->m_RTVs.resize(1);
        rt->m_Resources.resize(1);

        ID3D11Device1* d = g_Device->GetDevice();

        ID3D11Texture2D* d3d11FrameBuffer;
        DX_CALL(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer));
        rt->m_Resources[0] = new TextureResource2D(d3d11FrameBuffer, WINDOW_WIDTH, WINDOW_HEIGHT, TextureFormat::UNKNOWN, 1, 1, 0);

        DX_CALL(d->CreateRenderTargetView(d3d11FrameBuffer, 0, &rt->m_RTVs[0]));

        // Depth stencil
        const TextureFormat dsFormat = TextureFormat::R24G8_TYPELESS;
        const unsigned int dsCreationFlags = TRCF_BindDepthStencil;
        rt->m_DepthResource = new TextureResource2D(WINDOW_WIDTH, WINDOW_HEIGHT, dsFormat, 1, 1, dsCreationFlags);
        rt->m_DepthResource->Initialize();

        D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
        dsViewDesc.Format = ToDXGIFormatDSV(dsFormat);
        dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsViewDesc.Texture2D.MipSlice = 0;

        DX_CALL(d->CreateDepthStencilView(rt->m_DepthResource->GetResource(), &dsViewDesc, &rt->m_DSV));

        return rt;
    }

    GfxRenderTarget::GfxRenderTarget(unsigned int width, unsigned int height, unsigned int numRTs, bool useDepth, bool useStencil):
        m_NumRTs(numRTs)
    {
        m_RTVs.resize(numRTs);
        m_Resources.resize(numRTs);

        const TextureFormat texFormat = TextureFormat::RGBA_FLOAT;
        const unsigned int rtRcreationFlags = TRCF_BindRT | TRCF_BindSRV;
        ID3D11Device1* device = g_Device->GetDevice();

        for (size_t i = 0; i < numRTs; i++)
        {
            m_Resources[i] = new TextureResource2D(width, height, texFormat, 1, 1, rtRcreationFlags);
            m_Resources[i]->Initialize();

            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = ToDXGIFormat(texFormat);
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            renderTargetViewDesc.Texture2D.MipSlice = 0;

            DX_CALL(device->CreateRenderTargetView(m_Resources[i]->GetResource(), &renderTargetViewDesc, &m_RTVs[i]));
        }

        if (useDepth || useStencil)
        {
            ASSERT(useDepth, "[GfxRenderTarget] We can use stencil just with depth for now!");

            const unsigned int dsCreationFlags = TRCF_BindDepthStencil | TRCF_BindSRV;
            const TextureFormat dsFormat = useStencil ? TextureFormat::R24G8_TYPELESS : TextureFormat::R32_TYPELESS;
            
            // TODO: We need to use this format when making SRV to depth stencil
            //const DXGI_FORMAT DSRV_FORMAT = useStencil ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_R32_FLOAT;
            //const DXGI_FORMAT SSRV_FORMAT = DXGI_FORMAT_X24_TYPELESS_G8_UINT;

            m_DepthResource = new TextureResource2D(width, height, dsFormat, 1, 1, dsCreationFlags);
            m_DepthResource->Initialize();

            D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
            dsViewDesc.Format = ToDXGIFormatDSV(dsFormat);
            dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            dsViewDesc.Texture2D.MipSlice = 0;

            DX_CALL(device->CreateDepthStencilView(m_DepthResource->GetResource(), &dsViewDesc, &m_DSV));
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

    GfxCubemapRenderTarget::GfxCubemapRenderTarget(unsigned int width, unsigned int height)
    {
        const TextureFormat texFormat = TextureFormat::RGBA_FLOAT;
        const unsigned int rtCreationFlags = TRCF_BindRT | TRCF_BindCubemap | TRCF_BindSRV;

        ID3D11Device1* d = g_Device->GetDevice();

        m_Resource = new TextureResource2D(width, height, texFormat, 1, 6, rtCreationFlags);
        m_Resource->Initialize();

        for (size_t i = 0; i < 6; i++)
        {
            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = ToDXGIFormat(texFormat);
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            renderTargetViewDesc.Texture2DArray.MipSlice = 0;
            renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
            renderTargetViewDesc.Texture2DArray.ArraySize = 1;

            DX_CALL(d->CreateRenderTargetView(m_Resource->GetResource(), &renderTargetViewDesc, &m_RTVs[i]));
        }
    }

    GfxCubemapRenderTarget::~GfxCubemapRenderTarget()
    {
        m_Resource->Release();
        for (size_t i = 0; i < 6; i++) m_RTVs[i]->Release();
    }

    ///////////////////////////////////////
    //			Sampler                 //
    /////////////////////////////////////

    GfxSampler::GfxSampler(SamplerFilter filter, SamplerMode mode, Vec4 borderColor, float mipBias, float minMIP, float maxMIP, unsigned int maxAnisotropy)
    {
        // TODO: Add asserts for mip and anisotropy configurations

        const D3D11_TEXTURE_ADDRESS_MODE addressMode = GetDXMode(mode);

        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = GetDXFilter(filter);
        samplerDesc.AddressU = addressMode;
        samplerDesc.AddressV = addressMode;
        samplerDesc.AddressW = addressMode;
        samplerDesc.MipLODBias = mipBias;
        samplerDesc.MaxAnisotropy = maxAnisotropy;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.BorderColor[0] = borderColor.r;
        samplerDesc.BorderColor[1] = borderColor.g;
        samplerDesc.BorderColor[2] = borderColor.b;
        samplerDesc.BorderColor[3] = borderColor.a;
        samplerDesc.MinLOD = minMIP;
        samplerDesc.MaxLOD = maxMIP;

        DX_CALL(g_Device->GetDevice()->CreateSamplerState(&samplerDesc, &m_Sampler));
    }

    GfxSampler::~GfxSampler()
    {
        m_Sampler->Release();
    }

}