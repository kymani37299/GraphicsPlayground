#include "GfxTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <d3d11_1.h>

#include "core/GlobalVariables.h"
#include "gfx/GfxDevice.h"
#include "gfx/GfxResourceHelpers.h"

namespace GP
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

    namespace
    {
        DXGI_FORMAT ToDXGIViewFormat(TextureFormat format)
        {
            switch (format)
            {
            case TextureFormat::RGBA8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::RGBA_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case TextureFormat::R24G8_TYPELESS: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            case TextureFormat::R32_TYPELESS: return DXGI_FORMAT_R32_FLOAT;
            case TextureFormat::UNKNOWN: return DXGI_FORMAT_UNKNOWN;
            default: NOT_IMPLEMENTED;
            }

            return DXGI_FORMAT_UNKNOWN;
        }

        DXGI_FORMAT ToDXGIDepthFormat(TextureFormat format)
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
                CONSOLE_LOG("Failed to load texture: " + path);
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

    void TextureResource2D::Initialize(GfxContext* context)
    {
        m_RowPitch = m_Width * ToBPP(m_Format);
        m_SlicePitch = m_RowPitch * m_Height;

        D3D11_SUBRESOURCE_DATA* subresourceData = nullptr;
        void* texData[PathInitData::MAX_NUM_PATHS];
        bool freeMemoryAfter = false;
        bool uploadToTextureAfter = false;

        // If we have data paths defined load it here
        if (m_PathData.numPaths != 0) 
        {
            ASSERT(m_ArraySize == m_PathData.numPaths, "[TextureResource2D] If we are preloading textures array size must match with number of provided paths!");

            freeMemoryAfter = true;
            
            for (size_t i = 0; i < m_ArraySize; i++)
            {
                int width, height, bpp;
                texData[i] = LoadTexture(m_PathData.paths[i], width, height, bpp);

                // Fill the width and height data from first element
                if (i == 0) 
                {
                    m_Width = width;
                    m_Height = height;
                }
                ASSERT(texData[i], "[TextureResource2D] Error loading element data: " + m_PathData.paths[i]);
                //ASSERT(bpp == 4, "[TextureResource2D] Failed loading face data. We are only supporting RGBA8 textures.") TODO: Fix this, some texutres have bpp = 3 for some reason.
                ASSERT(m_Width == width && m_Height == height, "[TextureResource2D] Error: Face data size doesn't match with other faces : " + m_PathData.paths[i]);
            }

            m_RowPitch = m_Width * ToBPP(m_Format);
            m_SlicePitch = m_RowPitch * m_Height;

            if (m_NumMips == 1)
            {
                subresourceData = (D3D11_SUBRESOURCE_DATA*)malloc(m_ArraySize * sizeof(D3D11_SUBRESOURCE_DATA));
                for (size_t i = 0; i < m_ArraySize; i++)
                {
                    subresourceData[i].pSysMem = texData[i];
                    subresourceData[i].SysMemPitch = m_RowPitch;
                    subresourceData[i].SysMemSlicePitch = m_SlicePitch;
                }
            }
            else
            {
                // Prepare resource for mip generation
                AddCreationFlags(RCF_SRV | RCF_RT | RCF_GenerateMips);
                uploadToTextureAfter = true;
            }
        }

        const D3D11_TEXTURE2D_DESC textureDesc = FillTexture2DDescription(m_Width, m_Height, m_NumMips, m_ArraySize, m_NumSamples, ToDXGIFormat(m_Format), m_CreationFlags);
        DX_CALL(g_Device->GetDevice()->CreateTexture2D(&textureDesc, subresourceData, &m_Handle));

        if (uploadToTextureAfter)
        {
            for (size_t i = 0; i < m_ArraySize; i++) context->UploadToTexture(this, texData[i], i);
        }

        if (freeMemoryAfter)
        {
            for (size_t i = 0; i < m_ArraySize; i++)
            {
                FreeTexture(texData[i]);
            }
        }
    }

    TextureResource2D::~TextureResource2D()
    {
        ASSERT(m_RefCount == 0, "[~TextureResource2D] Trying to delete a referenced texture!");
        SAFE_RELEASE(m_Handle);
    }

    ///////////////////////////////////////////
    /// TextureResource3D                /////
    /////////////////////////////////////////

    void TextureResource3D::Initialize(GfxContext* context)
    {
        m_RowPitch = m_Width * ToBPP(m_Format);
        m_SlicePitch = m_RowPitch * m_Height;

        D3D11_TEXTURE3D_DESC textureDesc = Fill3DTextureDescription(m_Width, m_Height, m_Depth, m_NumMips, ToDXGIFormat(m_Format), m_CreationFlags);
        DX_CALL(g_Device->GetDevice()->CreateTexture3D(&textureDesc, nullptr, &m_Handle));

        // TODO: Data initialization
    }

    TextureResource3D::~TextureResource3D()
    {
        ASSERT(m_RefCount == 0, "[~TextureResource3D] Trying to delete a referenced texture!");
        SAFE_RELEASE(m_Handle);
    }

    ///////////////////////////////////////////
    /// GfxBaseTexture2D                 /////
    /////////////////////////////////////////

    template<>
    void GfxResource<TextureResource2D>::Initialize(GfxContext* context)
    {
        if (!m_Resource->Initialized()) m_Resource->Initialize(context);

        unsigned int creationFlags = m_Resource->GetCreationFlags();
        unsigned int numMips = m_Resource->GetNumMips();

        if (creationFlags & RCF_SRV)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = ToDXGIViewFormat(m_Resource->GetFormat());

            switch (m_Type)
            {
            case ResourceType::Texture2D:
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels = numMips == MAX_MIPS ? -1 : numMips;
                srvDesc.Texture2D.MostDetailedMip = 0;
                break;
            case ResourceType::Cubemap:
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
                srvDesc.TextureCube.MipLevels = numMips == MAX_MIPS ? -1 : numMips;
                srvDesc.TextureCube.MostDetailedMip = 0;
                break;
            case ResourceType::TextureArray2D:
            
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                srvDesc.Texture2DArray.MipLevels = numMips == MAX_MIPS ? -1 : numMips;
                srvDesc.Texture2DArray.MostDetailedMip = 0;
                srvDesc.Texture2DArray.FirstArraySlice = 0;
                srvDesc.Texture2DArray.ArraySize = m_Resource->GetArraySize();
                break;
            default: NOT_IMPLEMENTED;
            }

            DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(m_Resource->GetHandle(), &srvDesc, &m_SRV));
        }

        if (creationFlags & RCF_UAV)
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = ToDXGIViewFormat(m_Resource->GetFormat());

            switch (m_Type)
            {
            case ResourceType::Texture2D:
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Texture2D.MipSlice = 0;
                break;
            case ResourceType::Cubemap:
                ASSERT(0, "[GfxResource<TextureResource2D>::Initialize] Cannot create UAV from cubemap!");
            case ResourceType::TextureArray2D:
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
                uavDesc.Texture2DArray.MipSlice = 0;
                uavDesc.Texture2DArray.FirstArraySlice = 0;
                uavDesc.Texture2DArray.ArraySize = m_Resource->GetArraySize();
                break;
            default: NOT_IMPLEMENTED;
            }

            DX_CALL(g_Device->GetDevice()->CreateUnorderedAccessView(m_Resource->GetHandle(), &uavDesc, &m_UAV));
        }

        if (numMips != 1)
        {
            context->GenerateMips((GfxBaseTexture2D*)this);
        }
    }

    GfxBaseTexture2D::~GfxBaseTexture2D()
    {
        SAFE_RELEASE(m_Resource);
        SAFE_RELEASE(m_SRV);
        SAFE_RELEASE(m_UAV);
    }

    ///////////////////////////////////////////
    /// GfxBaseTexture3D                 /////
    /////////////////////////////////////////

    template<>
    void GfxResource<TextureResource3D>::Initialize(GfxContext* context)
    {
        if (!m_Resource->Initialized()) m_Resource->Initialize(context);

        unsigned int creationFlags = m_Resource->GetCreationFlags();
        unsigned int numMips = m_Resource->GetNumMips();

        if (creationFlags & RCF_SRV)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = ToDXGIViewFormat(m_Resource->GetFormat());

            switch (m_Type)
            {
            case ResourceType::Texture3D:
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
                srvDesc.Texture3D.MipLevels = numMips == MAX_MIPS ? -1 : numMips;
                srvDesc.Texture3D.MostDetailedMip = 0;
                break;
            default: NOT_IMPLEMENTED;
            }

            DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(m_Resource->GetHandle(), &srvDesc, &m_SRV));
        }

        if (creationFlags & RCF_UAV)
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = ToDXGIViewFormat(m_Resource->GetFormat());

            switch (m_Type)
            {
            case ResourceType::Texture3D:
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
                uavDesc.Texture3D.MipSlice = 0;
                uavDesc.Texture3D.FirstWSlice = 0;
                uavDesc.Texture3D.WSize = m_Resource->GetDepth();
                break;
            default: NOT_IMPLEMENTED;
            }

            DX_CALL(g_Device->GetDevice()->CreateUnorderedAccessView(m_Resource->GetHandle(), &uavDesc, &m_UAV));
        }
    }

    GfxBaseTexture3D::~GfxBaseTexture3D()
    {
        SAFE_RELEASE(m_Resource);
        SAFE_RELEASE(m_SRV);
        SAFE_RELEASE(m_UAV);
    }

    ///////////////////////////////////////////
    /// RenderTarget                     /////
    /////////////////////////////////////////

    GfxRenderTarget* GfxRenderTarget::CreateFromSwapChain(IDXGISwapChain1* swapchain)
    {
        RenderTargetConfig swapchainConfig;
        swapchainConfig.Width = GlobalVariables::GP_CONFIG.WindowWidth;
        swapchainConfig.Height = GlobalVariables::GP_CONFIG.WindowHeight;
        swapchainConfig.NumRenderTargets = 1;
        swapchainConfig.NumSamples = 1;
        swapchainConfig.Format = TextureFormat::UNKNOWN;
        swapchainConfig.UseDepth = true;
        swapchainConfig.UseStencil = true;

        GfxRenderTarget* rt = new GfxRenderTarget();
        rt->m_Config = swapchainConfig;
        rt->m_RTVs.resize(1);
        rt->m_Resources.resize(1);
        rt->m_Initialized = true;

        ID3D11Device1* d = g_Device->GetDevice();

        ID3D11Texture2D* d3d11FrameBuffer;
        DX_CALL(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer));
        rt->m_Resources[0] = new TextureResource2D(d3d11FrameBuffer, swapchainConfig.Width, swapchainConfig.Height, swapchainConfig.Format, 1, 1, swapchainConfig.NumSamples, 0);

        DX_CALL(d->CreateRenderTargetView(d3d11FrameBuffer, 0, &rt->m_RTVs[0]));

        // Depth stencil
        const TextureFormat dsFormat = TextureFormat::R24G8_TYPELESS;
        const unsigned int dsCreationFlags = RCF_DS;
        rt->m_DepthResource = new TextureResource2D(swapchainConfig.Width, swapchainConfig.Height, dsFormat, 1, 1, 1, dsCreationFlags);
        rt->m_DepthResource->Initialize(g_Device->GetImmediateContext());

        D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
        dsViewDesc.Format = ToDXGIDepthFormat(dsFormat);
        dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsViewDesc.Texture2D.MipSlice = 0;

        DX_CALL(d->CreateDepthStencilView(rt->m_DepthResource->GetHandle(), &dsViewDesc, &rt->m_DSV));

        return rt;
    }

    void GfxRenderTarget::Initialize(GfxContext* context)
    {
        ID3D11Device1* device = g_Device->GetDevice();

        for (size_t i = 0; i < m_Config.NumRenderTargets; i++)
        {
            m_Resources[i]->Initialize(context);

            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = ToDXGIViewFormat(m_Resources[i]->GetFormat());
            renderTargetViewDesc.ViewDimension = m_Config.NumSamples == 1 ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS;
            renderTargetViewDesc.Texture2D.MipSlice = 0;

            DX_CALL(device->CreateRenderTargetView(m_Resources[i]->GetHandle(), &renderTargetViewDesc, &m_RTVs[i]));
        }

        if (m_Config.UseDepth || m_Config.UseStencil)
        {
            ASSERT(m_DepthResource, "[GfxRenderTarget] Internal error!");

            m_DepthResource->Initialize(context);

            D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
            dsViewDesc.Format = ToDXGIDepthFormat(m_DepthResource->GetFormat());
            dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            dsViewDesc.Texture2D.MipSlice = 0;

            DX_CALL(device->CreateDepthStencilView(m_DepthResource->GetHandle(), &dsViewDesc, &m_DSV));
        }

        m_Initialized = true;
    }

    void GfxRenderTarget::InitResources()
    {
        ASSERT(m_Config.Width != 0 && m_Config.Height != 0, "[GfxRenderTarget] Cannot set width or height of rt to 0!");

        m_RTVs.resize(m_Config.NumRenderTargets);
        m_Resources.resize(m_Config.NumRenderTargets);

        for (size_t i = 0; i < m_Config.NumRenderTargets; i++)
            m_Resources[i] = new TextureResource2D(m_Config.Width, m_Config.Height, m_Config.Format, 1, 1, m_Config.NumSamples, DEFAULT_RT_FLAGS);

        if (m_Config.UseDepth || m_Config.UseStencil)
        {
            ASSERT(m_Config.UseDepth, "[GfxRenderTarget] We can use stencil just with depth for now!");
            m_DepthResource = new TextureResource2D(m_Config.Width, m_Config.Height, m_Config.UseStencil ? DEFAULT_DS_STENCIL_FORMAT : DEFAULT_DS_FORMAT, 1, 1, 1, DEFAULT_DS_FLAGS);
        }

        m_Initialized = false;
    }

    void GfxRenderTarget::FreeResources()
    {
        for (unsigned int i = 0; i < m_Config.NumRenderTargets; i++)
        {
            m_RTVs[i]->Release();
            m_Resources[i]->Release();
        }

        SAFE_RELEASE(m_DSV);
        SAFE_RELEASE(m_DepthResource);

        m_Initialized = false;
    }

    ///////////////////////////////////////////
    /// CubemapRenderTarget            /////
    /////////////////////////////////////////

    void GfxCubemapRenderTarget::Initialize(GfxContext* context)
    {
        m_Resource->Initialize(context);
        for (size_t i = 0; i < 6; i++)
        {
            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = ToDXGIViewFormat(m_Resource->GetFormat());
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            renderTargetViewDesc.Texture2DArray.MipSlice = 0;
            renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
            renderTargetViewDesc.Texture2DArray.ArraySize = 1;

            DX_CALL(g_Device->GetDevice()->CreateRenderTargetView(m_Resource->GetHandle(), &renderTargetViewDesc, &m_RTVs[i]));
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
        // TODO: Deferred initialization

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