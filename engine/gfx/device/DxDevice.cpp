#include "gfx/device/DxDevice.h"

#ifdef DEBUG_GFX
#include <dxgidebug.h>
#include "util/StringUtil.h"
#endif

#include "core/Core.h"
#include "core/Window.h"

#include "gfx/device/DXCommon.h"
#include "gfx/device/DxShader.h"
#include "gfx/device/DxBuffers.h"
#include "gfx/device/DxTexture.h"
#include "gfx/device/DxRenderTarget.h"

static DxVertexBuffer* QUAD_BUFFER = nullptr;

namespace
{
    inline D3D11_STENCIL_OP sop2desc(StencilOp op)
    {
        switch (op)
        {
        case Discard:
            return D3D11_STENCIL_OP_ZERO;
        case Keep:
            return D3D11_STENCIL_OP_KEEP;
        case Replace:
            return D3D11_STENCIL_OP_REPLACE;
        default:
            NOT_IMPLEMENTED;
        }

        return D3D11_STENCIL_OP_ZERO;
    }

    inline D3D11_COMPARISON_FUNC GetD3D11Comparison(CompareOp op)
    {
        switch (op)
        {
        case Always:
            return D3D11_COMPARISON_ALWAYS;
        case Equals:
            return D3D11_COMPARISON_EQUAL;
        case Less:
            return D3D11_COMPARISON_LESS;
        default:
            NOT_IMPLEMENTED;
        }

        return D3D11_COMPARISON_ALWAYS;
    }

    inline D3D11_DEPTH_STENCILOP_DESC GetD3D11Desc(StencilOp fail, StencilOp depthFail, StencilOp pass, CompareOp compare)
    {
        return { sop2desc(fail) , sop2desc(depthFail), sop2desc(pass), GetD3D11Comparison(compare) };
    }
}

DxDeviceState::~DxDeviceState()
{
    SAFE_RELEASE(m_RasterizerState);
    SAFE_RELEASE(m_DepthStencilState);
    SAFE_RELEASE(m_BlendState);
}

void DxDeviceState::Compile(DxDevice* device)
{
    ASSERT(!m_Compiled, "[DxDeviceState] Already compiled this state!");

    ID3D11Device1* d = device->GetDevice();

    // Rasterizer
    D3D11_RASTERIZER_DESC1 rDesc = {};
    rDesc.FillMode = D3D11_FILL_SOLID;
    rDesc.CullMode = m_BackfaceCullingEnabled ? D3D11_CULL_BACK : D3D11_CULL_NONE;
    rDesc.FrontCounterClockwise = true;
    rDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
    rDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
    rDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rDesc.DepthClipEnable = true;
    rDesc.ScissorEnable = false;
    rDesc.MultisampleEnable = false;
    rDesc.AntialiasedLineEnable = false;
    rDesc.ForcedSampleCount = 0;
    DX_CALL(d->CreateRasterizerState1(&rDesc, &m_RasterizerState));

    //m_DeviceContext->RSSetState(m_RasterizerState);
    //m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState, m_State.stencilRef);
    //const FLOAT blendFactor[] = { 1.0f,1.0f,1.0f,1.0f };
    //m_DeviceContext->OMSetBlendState(m_BlendState, blendFactor, 0xffffffff);

    // Depth
    CD3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable = m_DepthEnabled;
    dsDesc.DepthWriteMask = m_DepthEnabled && m_DepthWriteEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = GetD3D11Comparison(m_DepthCompareOp);

    const D3D11_DEPTH_STENCILOP_DESC stencilOp = GetD3D11Desc(m_StencilOp[0], m_StencilOp[1], m_StencilOp[2], m_StencilCompareOp);
    dsDesc.StencilEnable = m_StencilEnabled;
    dsDesc.StencilReadMask = m_StencilRead;
    dsDesc.StencilWriteMask = m_StencilWrite;
    dsDesc.FrontFace = stencilOp;
    dsDesc.BackFace = stencilOp;
    DX_CALL(d->CreateDepthStencilState(&dsDesc, &m_DepthStencilState));

    // Blend
    D3D11_RENDER_TARGET_BLEND_DESC1 rtbDesc = {};
    rtbDesc.BlendEnable = m_AlphaBlendEnabled;
    rtbDesc.LogicOpEnable = false;
    rtbDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    rtbDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rtbDesc.BlendOp = D3D11_BLEND_OP_ADD;
    rtbDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbDesc.DestBlendAlpha = D3D11_BLEND_ONE;
    rtbDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbDesc.LogicOp = D3D11_LOGIC_OP_NOOP;
    rtbDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    D3D11_BLEND_DESC1 bDesc = {};
    bDesc.AlphaToCoverageEnable = false;
    bDesc.IndependentBlendEnable = false;
    bDesc.RenderTarget[0] = rtbDesc;

    DX_CALL(d->CreateBlendState1(&bDesc, &m_BlendState));

    m_Compiled = true;
}

DxDevice::DxDevice(Window* window)
{
    if(!CreateDevice()) return;

#ifdef DEBUG_GFX
    InitDebugLayer();
#endif

    CreateSwapChain(window);

    InitContext(window);
    InitSamplers();
    DxBufferAllocator::Init(this);

    m_Initialized = true;

    float quadVertices[] = { // x, y, u, v
                        -1.0f,  1.0f, 0.f, 0.f,
                        1.0f, -1.0f, 1.f, 1.f,
                        -1.0f, -1.0f, 0.f, 1.f,
                        -1.0f,  1.0f, 0.f, 0.f,
                        1.0f,  1.0f, 1.f, 0.f,
                        1.0f, -1.0f, 1.f, 1.f
    };

    VertexBufferData quadData = {};
    quadData.numBytes = sizeof(float) * 4 * 6;
    quadData.stride = sizeof(float) * 4;
    quadData.pData = quadVertices;

    QUAD_BUFFER = DxBufferAllocator::Get()->AllocateVertexBuffer(this, quadData);
}

DxDevice::~DxDevice()
{
    DxBufferAllocator::Deinit();
    delete m_FinalRT;
    m_PointBorderSampler->Release();
    m_SwapChain->Release();
    m_DeviceContext->Release();
    m_Device->Release();;

#ifdef DEBUG_GFX
    ID3D11Debug* d3dDebug = nullptr;
    m_Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
    d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
#endif
}

void DxDevice::Clear(const Vec4& color)
{
    const FLOAT clearColor[4] = { color.x, color.y, color.z, color.w };

    if (m_RenderTarget)
    {
        for (size_t i = 0; i < m_RenderTarget->GetNumRTs(); i++)
        {
            m_DeviceContext->ClearRenderTargetView(m_RenderTarget->GetRTView(i), clearColor);
        }
    }

    if (m_DepthStencil)
    {
        m_DeviceContext->ClearDepthStencilView(m_DepthStencil->GetDSView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }
}

void DxDevice::BindState(DxDeviceState* state)
{
    ASSERT(!state || state->IsCompiled(), "Trying to bind state that isn't compiled!");

    m_State = state ? state : &m_DefaultState;

    const FLOAT blendFactor[] = { 1.0f,1.0f,1.0f,1.0f };
    m_DeviceContext->RSSetState(m_State->GetRasterizerState());
    m_DeviceContext->OMSetDepthStencilState(m_State->GetDepthStencilState(), m_StencilRef);
    m_DeviceContext->OMSetBlendState(m_State->GetBlendState(), blendFactor, 0xffffffff);
}

void DxDevice::BindConstantBuffer(ShaderStage stage, ID3D11Buffer* constantBuffer, unsigned int binding)
{
    if (stage & VS)
        m_DeviceContext->VSSetConstantBuffers(binding, 1, &constantBuffer);

    if (stage & PS)
        m_DeviceContext->PSSetConstantBuffers(binding, 1, &constantBuffer);
}

void DxDevice::BindStructuredBuffer(ShaderStage stage, ID3D11ShaderResourceView* structuredBufferSrv, unsigned int binding)
{
    if (stage & VS)
        m_DeviceContext->VSSetShaderResources(binding, 1, &structuredBufferSrv);

    if(stage & PS)
        m_DeviceContext->PSSetShaderResources(binding, 1, &structuredBufferSrv);
}

void DxDevice::BindIndexBuffer(DxIndexBuffer* indexBuffer)
{
    m_DeviceContext->IASetIndexBuffer(indexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, indexBuffer->GetOffset());
}

void DxDevice::BindVertexBuffer(DxVertexBuffer* vertexBuffer)
{
    unsigned int stride = vertexBuffer->GetStride();
    unsigned int offset = vertexBuffer->GetOffset();
    ID3D11Buffer* buffer = vertexBuffer->GetBuffer();

    m_DeviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
}

inline void DX_BindTexture(ID3D11DeviceContext1* context, ShaderStage stage, ID3D11ShaderResourceView* srv, unsigned int binding)
{
    if (stage & VS)
        context->VSSetShaderResources(binding, 1, &srv);

    if (stage & PS)
        context->PSSetShaderResources(binding, 1, &srv);
}

inline void DX_UnbindTexture(ID3D11DeviceContext1* context, ShaderStage stage, unsigned int binding)
{
    static ID3D11ShaderResourceView* nullSRV[1] = { nullptr };

    if (stage & VS)
        context->VSSetShaderResources(binding, 1, nullSRV);

    if (stage & PS)
        context->PSSetShaderResources(binding, 1, nullSRV);
}

void DxDevice::BindTexture(ShaderStage stage, DxTexture* texture, unsigned int binding)
{
    DX_BindTexture(m_DeviceContext, stage, texture->GetTextureView(), binding);
}

void DxDevice::BindTexture(ShaderStage stage, DxRenderTarget* renderTarget, unsigned int binding, unsigned int texIndex)
{
    ID3D11ShaderResourceView* srv = texIndex == -1 ? renderTarget->GetDSSRView() : renderTarget->GetSRView(texIndex);
    DX_BindTexture(m_DeviceContext, stage, srv, binding);
}

void DxDevice::BindTexture(ShaderStage stage, DxCubemapRenderTarget* cubemapRT, unsigned int binding)
{
    DX_BindTexture(m_DeviceContext, stage, cubemapRT->GetSRView(), binding);
}

void DxDevice::UnbindTexture(ShaderStage stage, unsigned int binding)
{
    DX_UnbindTexture(m_DeviceContext, stage, binding);
}

void DxDevice::BindShader(DxShader* shader)
{
    m_DeviceContext->IASetInputLayout(shader->GetInputLayout());
    m_DeviceContext->VSSetShader(shader->GetVertexShader(), nullptr, 0);
    m_DeviceContext->PSSetShader(shader->GetPixelShader(), nullptr, 0);
}

void DX_SetRenderTarget(ID3D11DeviceContext1* context, unsigned int numRTs, ID3D11RenderTargetView** rtvs, ID3D11DepthStencilView* dsv, int width, int height)
{
    const D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (float) width, (float) height, 0.0f, 1.0f };
    if (width > 0)
        context->RSSetViewports(1, &viewport);
    context->OMSetRenderTargets(numRTs, rtvs, dsv);
}

void DxDevice::SetRenderTarget(DxCubemapRenderTarget* cubemapRT, unsigned int face)
{
    m_RenderTarget = nullptr;
    m_DepthStencil = nullptr;

    ID3D11RenderTargetView* rtv = cubemapRT->GetRTView(face);
    DX_SetRenderTarget(m_DeviceContext, 1, &rtv, nullptr, cubemapRT->GetWidth(), cubemapRT->GetHeight());
}

void DxDevice::SetRenderTarget(DxRenderTarget* renderTarget)
{
    m_RenderTarget = renderTarget;

    unsigned int numRTs = m_RenderTarget ? m_RenderTarget->GetNumRTs() : 0;
    ID3D11RenderTargetView** rtvs = m_RenderTarget ? m_RenderTarget->GetRTViews() : nullptr;
    ID3D11DepthStencilView* dsv = m_DepthStencil ? m_DepthStencil->GetDSView() : nullptr;
    int width = -1;
    int height = -1;

    if (m_RenderTarget != nullptr)
    {
        width = m_RenderTarget->GetWidth();
        height = m_RenderTarget->GetHeight();
    }

    if (m_DepthStencil != nullptr)
    {
        width = m_DepthStencil->GetWidth();
        height = m_DepthStencil->GetHeight();
    }

    DX_SetRenderTarget(m_DeviceContext, numRTs, rtvs, dsv, width, height);
}

void DxDevice::SetDepthStencil(DxRenderTarget* depthStencil)
{
    m_DepthStencil = depthStencil;
    SetRenderTarget(m_RenderTarget);
}

void DxDevice::SetStencilRef(unsigned int ref)
{
    m_StencilRef = ref;
    m_DeviceContext->OMSetDepthStencilState(m_State->GetDepthStencilState(), m_StencilRef);
}

void DxDevice::Draw(unsigned int numVerts)
{
    m_DeviceContext->Draw(numVerts, 0);
}

void DxDevice::DrawIndexed(unsigned int numIndices)
{
    m_DeviceContext->DrawIndexed(numIndices, 0, 0);
}

void DxDevice::DrawFullSceen()
{
    BindVertexBuffer(QUAD_BUFFER);
    Draw(6);
}

void DxDevice::BeginPass(const std::string& debugName)
{
#ifdef DEBUG
    std::wstring wDebugName = StringUtil::ToWideString(debugName);
    m_DebugMarkers->BeginEvent(wDebugName.c_str());
#endif
}

void DxDevice::EndPass()
{
#ifdef DEBUG
    m_DebugMarkers->EndEvent();
#endif
}

void DxDevice::Present()
{
    m_SwapChain->Present(1, 0);
}

bool DxDevice::CreateDevice()
{
    ID3D11Device* baseDevice;
    ID3D11DeviceContext* baseDeviceContext;
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef DEBUG_GFX
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
        0, creationFlags,
        featureLevels, ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION, &baseDevice,
        0, &baseDeviceContext);

    if (FAILED(hr)) {
        MessageBoxA(0, "D3D11CreateDevice() failed", "Fatal Error", MB_OK);
        return false;
    }

    DX_CALL(baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&m_Device));
    baseDevice->Release();

    DX_CALL(baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&m_DeviceContext));
    baseDeviceContext->Release();

    return true;
}

void DxDevice::InitDebugLayer()
{
    ID3D11Debug* d3dDebug = nullptr;
    m_Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
    if (d3dDebug)
    {
        ID3D11InfoQueue* d3dInfoQueue = nullptr;
        if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
        {
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
            d3dInfoQueue->Release();
        }
        d3dDebug->Release();
    }

    DX_CALL(m_DeviceContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&m_DebugMarkers));
}

void DxDevice::CreateSwapChain(Window* window)
{
    IDXGIFactory2* dxgiFactory;
    {
        IDXGIDevice1* dxgiDevice;
        DX_CALL(m_Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice));

        IDXGIAdapter* dxgiAdapter;
        DX_CALL(dxgiDevice->GetAdapter(&dxgiAdapter));
        dxgiDevice->Release();

        DXGI_ADAPTER_DESC adapterDesc;
        dxgiAdapter->GetDesc(&adapterDesc);

        OutputDebugStringA("Graphics Device: ");
        OutputDebugStringW(adapterDesc.Description);

        DX_CALL(dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory));
        dxgiAdapter->Release();
    }

    DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
    d3d11SwapChainDesc.Width = window->GetWidth();
    d3d11SwapChainDesc.Height = window->GetHeight();
    d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    d3d11SwapChainDesc.SampleDesc.Count = 1;
    d3d11SwapChainDesc.SampleDesc.Quality = 0;
    d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    d3d11SwapChainDesc.BufferCount = 2;
    d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    d3d11SwapChainDesc.Flags = 0;

    DX_CALL(dxgiFactory->CreateSwapChainForHwnd(m_Device, window->GetHandle(), &d3d11SwapChainDesc, 0, 0, &m_SwapChain));

    dxgiFactory->Release();

    m_FinalRT = DxRenderTarget::CreateFromSwapChain(this, m_SwapChain);
}

void DxDevice::InitContext(Window* window)
{
    m_DefaultState.Compile(this);
    BindState(&m_DefaultState);
    SetRenderTarget(m_FinalRT);
    SetDepthStencil(m_FinalRT);

    m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DxDevice::InitSamplers()
{
    D3D11_SAMPLER_DESC samplerDesc = {};

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    DX_CALL(m_Device->CreateSamplerState(&samplerDesc, &m_PointBorderSampler));

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    DX_CALL(m_Device->CreateSamplerState(&samplerDesc, &m_LinearBorderSampler));

    m_DeviceContext->VSSetSamplers(0, 1, &m_PointBorderSampler);
    m_DeviceContext->PSSetSamplers(0, 1, &m_PointBorderSampler);

    m_DeviceContext->VSSetSamplers(1, 1, &m_LinearBorderSampler);
    m_DeviceContext->PSSetSamplers(1, 1, &m_LinearBorderSampler);
}
