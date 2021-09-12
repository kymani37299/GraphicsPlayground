#include "DxRenderTarget.h"

#include "core/Core.h"
#include "gfx/device/DxCommon.h"
#include "gfx/device/DxDevice.h"

///////////////////////////////////////////
/// DxRenderTarget                   /////
/////////////////////////////////////////

DxRenderTarget* DxRenderTarget::CreateFromSwapChain(DxDevice* device, IDXGISwapChain1* swapchain)
{
    DxRenderTarget* rt = new DxRenderTarget();
    rt->m_NumRTs = 1;
    rt->m_RTViews.resize(1);
    rt->m_SRViews.resize(1);
    rt->m_SRViews[0] = nullptr;
    rt->m_DSSRView = nullptr;
    rt->m_Width = WINDOW_WIDTH;
    rt->m_Height = WINDOW_HEIGHT;

    ID3D11Device1* d = device->GetDevice();

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

DxRenderTarget::DxRenderTarget(DxDevice* device, const RenderTargetDesc& desc)
{
    m_NumRTs = desc.numRTs;
    m_Width = desc.width;
    m_Height = desc.height;

    CreateRenderTargets(device->GetDevice(), desc);

    if (desc.useDepth)
    {
        CreateDepthStencil(device->GetDevice(), desc);
    }
}

DxRenderTarget::~DxRenderTarget()
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

void DxRenderTarget::CreateRenderTargets(ID3D11Device1* device, const RenderTargetDesc& desc)
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

void DxRenderTarget::CreateDepthStencil(ID3D11Device1* device, const RenderTargetDesc& desc)
{
    ASSERT(desc.useDepth , "[DxRenderTarget::CreateDepthStencil] desc.useDepth == false");

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
/// DxCubemapRenderTarget            /////
/////////////////////////////////////////

DxCubemapRenderTarget::DxCubemapRenderTarget(DxDevice* device, const RenderTargetDesc& desc) :
    m_Width(desc.width),
    m_Height(desc.height)
{
    ID3D11Device1* d = device->GetDevice();

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

DxCubemapRenderTarget::~DxCubemapRenderTarget()
{
    SAFE_RELEASE(m_TextureMap);
    m_SRView->Release();
    for (size_t i = 0; i < 6; i++) m_RTViews[i]->Release();
}