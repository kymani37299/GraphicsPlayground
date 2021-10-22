#include "GfxCore.h"

#include <d3d11_1.h>

#ifdef DEBUG
#include <dxgidebug.h>
#include "util/StringUtil.h"
#endif

#include "Common.h"
#include "core/Window.h"
#include "gui/GUI.h"
#include "gfx/GfxDefaultsData.h"
#include "gfx/ShaderFactory.h"

namespace GP
{
    GfxDevice* g_Device = nullptr;

    namespace
    {
        inline D3D11_STENCIL_OP sop2desc(StencilOp op)
        {
            switch (op)
            {
            case StencilOp::Discard:
                return D3D11_STENCIL_OP_ZERO;
            case StencilOp::Keep:
                return D3D11_STENCIL_OP_KEEP;
            case StencilOp::Replace:
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
            case CompareOp::Always:
                return D3D11_COMPARISON_ALWAYS;
            case CompareOp::Equals:
                return D3D11_COMPARISON_EQUAL;
            case CompareOp::Less:
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

    ///////////////////////////////////////
    //			Defaults		        //
    /////////////////////////////////////

    namespace GfxDefaults
    {
        GfxVertexBuffer* VB_CUBE = nullptr;
        GfxVertexBuffer* VB_2DQUAD = nullptr;
        GfxVertexBuffer* VB_QUAD = nullptr;

        void InitDefaults()
        {
            VB_CUBE = new GfxVertexBuffer(Data::VB_CUBE_DATA());
            VB_2DQUAD = new GfxVertexBuffer(Data::VB_2DQUAD_DATA());
            VB_QUAD = new GfxVertexBuffer(Data::VB_QUAD_DATA());
        }

        void DestroyDefaults()
        {
            SAFE_DELETE(VB_CUBE);
            SAFE_DELETE(VB_2DQUAD);
            SAFE_DELETE(VB_QUAD);
        }
    }


    ///////////////////////////////////////
    //			DeviceState		        //
    /////////////////////////////////////

    GfxDeviceState::~GfxDeviceState()
    {
        SAFE_RELEASE(m_RasterizerState);
        SAFE_RELEASE(m_DepthStencilState);
        SAFE_RELEASE(m_BlendState);
    }

    void GfxDeviceState::Compile()
    {
        ASSERT(!m_Compiled, "[GfxDeviceState] Already compiled this state!");

        ID3D11Device1* d = g_Device->GetDevice();

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

    ///////////////////////////////////////
    //			Device  		        //
    /////////////////////////////////////

    GfxDevice::GfxDevice()
    {
        if (!CreateDevice())
        {
            m_Device = nullptr;
        }
    }

    void GfxDevice::Init()
    {
        ASSERT(m_Device, "Failed to create device!");

#ifdef DEBUG
        InitDebugLayer();
#endif

        CreateSwapChain();

        InitContext();
        InitSamplers();

        m_ShaderFactory = new ShaderFactory(this);

        m_Initialized = true;

        GfxDefaults::InitDefaults();

        g_GUI = new GUI(Window::Get()->GetHandle(), m_Device, m_DeviceContext);
    }

    GfxDevice::~GfxDevice()
    {
        GfxDefaults::DestroyDefaults();
        delete m_ShaderFactory;
        delete m_FinalRT;
        for (ID3D11SamplerState* sampler : m_Samplers) sampler->Release();
        m_SwapChain->Release();
        m_DeviceContext->Release();
        m_Device->Release();
        m_DefaultState.~GfxDeviceState();

        ClearPipeline();

#ifdef DEBUG
        ID3D11Debug* d3dDebug = nullptr;
        m_Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
        d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_IGNORE_INTERNAL | D3D11_RLDO_DETAIL);
#endif
    }

    void GfxDevice::Clear(const Vec4& color)
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

    void GfxDevice::BindState(GfxDeviceState* state)
    {
        ASSERT(!state || state->IsCompiled(), "Trying to bind state that isn't compiled!");

        m_State = state ? state : &m_DefaultState;

        const FLOAT blendFactor[] = { 1.0f,1.0f,1.0f,1.0f };
        m_DeviceContext->RSSetState(m_State->GetRasterizerState());
        m_DeviceContext->OMSetDepthStencilState(m_State->GetDepthStencilState(), m_StencilRef);
        m_DeviceContext->OMSetBlendState(m_State->GetBlendState(), blendFactor, 0xffffffff);
    }

    static inline ID3D11Buffer* GetDeviceBuffer(GfxBufferResource* bufferResource)
    {
        if (!bufferResource->Initialized())
            bufferResource->Initialize();

        ASSERT(bufferResource->GetBuffer(), "bufferResource->GetBuffer() == nullptr");
        return bufferResource->GetBuffer();
    }

    static inline ID3D11ShaderResourceView* GetDeviceSRV(GfxBufferResource* bufferResource)
    {
        if (!bufferResource->Initialized())
            bufferResource->Initialize();

        ASSERT(bufferResource->GetSRV(), "bufferResource->GetSRV() == nullptr");
        return bufferResource->GetSRV();
    }

    static inline ID3D11UnorderedAccessView* GetDeviceUAV(GfxBufferResource* bufferResource)
    {
        if (!bufferResource->Initialized())
            bufferResource->Initialize();

        ASSERT(bufferResource->GetUAV(), "bufferResource->GetUAV() == nullptr");
        return bufferResource->GetUAV();
    }

    void GfxDevice::BindIndexBuffer(GfxIndexBuffer* indexBuffer)
    {
        unsigned int offset = indexBuffer ? indexBuffer->GetOffset() : 0;
        ID3D11Buffer* buffer = indexBuffer ? GetDeviceBuffer(indexBuffer->GetBufferResource()) : nullptr;

        m_DeviceContext->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, offset);
    }

    void GfxDevice::BindVertexBuffer(GfxVertexBuffer* vertexBuffer)
    {
        unsigned int stride = vertexBuffer ? vertexBuffer->GetStride() : 0;
        unsigned int offset = vertexBuffer ? vertexBuffer->GetOffset() : 0;
        ID3D11Buffer* buffer = vertexBuffer ? GetDeviceBuffer(vertexBuffer->GetBufferResource()) : nullptr;

        m_DeviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
    }

    void GfxDevice::BindConstantBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
    {
        ID3D11Buffer* buffer = nullptr;

        if (gfxBuffer)
        {
            gfxBuffer->GetBufferResource()->CheckForFlags(BCF_ConstantBuffer);
            buffer = GetDeviceBuffer(gfxBuffer->GetBufferResource());
        }

        if (shaderStage & VS)
            m_DeviceContext->VSSetConstantBuffers(binding, 1, &buffer);

        if (shaderStage & GS)
            m_DeviceContext->GSSetConstantBuffers(binding, 1, &buffer);

        if (shaderStage & PS)
            m_DeviceContext->PSSetConstantBuffers(binding, 1, &buffer);

        if (shaderStage & CS)
            m_DeviceContext->CSSetConstantBuffers(binding, 1, &buffer);
    }

    void GfxDevice::BindStructuredBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
    {
        ID3D11ShaderResourceView* srv = nullptr;

        if (gfxBuffer)
        {
            gfxBuffer->GetBufferResource()->CheckForFlags(BCF_StructuredBuffer);
            srv = GetDeviceSRV(gfxBuffer->GetBufferResource());
        }

        if (shaderStage & VS)
            m_DeviceContext->VSSetShaderResources(binding, 1, &srv);

        if (shaderStage & GS)
            m_DeviceContext->GSSetShaderResources(binding, 1, &srv);

        if (shaderStage & PS)
            m_DeviceContext->PSSetShaderResources(binding, 1, &srv);

        if (shaderStage & CS)
            m_DeviceContext->CSSetShaderResources(binding, 1, &srv);
    }

    void GfxDevice::BindRWStructuredBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
    {
        ASSERT(shaderStage & CS, "Binding UAV is only supported by CS");

        ID3D11UnorderedAccessView* uav = nullptr;
        if (gfxBuffer)
        {
            gfxBuffer->GetBufferResource()->CheckForFlags(BCF_StructuredBuffer | BCF_UAV);
            uav = GetDeviceUAV(gfxBuffer->GetBufferResource());
        }

        m_DeviceContext->CSSetUnorderedAccessViews(binding, 1, &uav, nullptr);
    }

    inline void DX_BindTexture(ID3D11DeviceContext1* context, unsigned int shaderStage, ID3D11ShaderResourceView* srv, unsigned int binding)
    {
        if (shaderStage & VS)
            context->VSSetShaderResources(binding, 1, &srv);

        if (shaderStage & GS)
            context->GSSetShaderResources(binding, 1, &srv);

        if (shaderStage & PS)
            context->PSSetShaderResources(binding, 1, &srv);

        if (shaderStage & CS)
            context->CSSetShaderResources(binding, 1, &srv);
    }

    inline void DX_UnbindTexture(ID3D11DeviceContext1* context, unsigned int shaderStage, unsigned int binding)
    {
        static ID3D11ShaderResourceView* nullSRV[1] = { nullptr };

        if (shaderStage & VS)
            context->VSSetShaderResources(binding, 1, nullSRV);

        if (shaderStage & GS)
            context->GSSetShaderResources(binding, 1, nullSRV);

        if (shaderStage & PS)
            context->PSSetShaderResources(binding, 1, nullSRV);

        if (shaderStage & CS)
            context->CSSetShaderResources(binding, 1, nullSRV);
    }

    void GfxDevice::BindTexture(unsigned int shaderStage, GfxTexture* texture, unsigned int binding)
    {
        DX_BindTexture(m_DeviceContext, shaderStage, texture->GetTextureView(), binding);
    }

    void GfxDevice::BindTexture(unsigned int shaderStage, GfxRenderTarget* renderTarget, unsigned int binding, unsigned int texIndex)
    {
        ID3D11ShaderResourceView* srv = texIndex == -1 ? renderTarget->GetDSSRView() : renderTarget->GetSRView(texIndex);
        DX_BindTexture(m_DeviceContext, shaderStage, srv, binding);
    }

    void GfxDevice::BindTexture(unsigned int shaderStage, GfxCubemapRenderTarget* cubemapRT, unsigned int binding)
    {
        DX_BindTexture(m_DeviceContext, shaderStage, cubemapRT->GetSRView(), binding);
    }

    void GfxDevice::UnbindTexture(unsigned int shaderStage, unsigned int binding)
    {
        DX_UnbindTexture(m_DeviceContext, shaderStage, binding);
    }

    void GfxDevice::BindShader(GfxShader* shader)
    {
        ID3D11InputLayout* il = shader ? shader->GetInputLayout() : nullptr;
        ID3D11VertexShader* vs = shader ? shader->GetVertexShader() : nullptr;
        ID3D11PixelShader* ps = shader ? shader->GetPixelShader() : nullptr;

        m_DeviceContext->IASetInputLayout(il);
        m_DeviceContext->VSSetShader(vs, nullptr, 0);
        m_DeviceContext->PSSetShader(ps, nullptr, 0);
    }

    void GfxDevice::BindShader(GfxComputeShader* shader)
    {
        ID3D11ComputeShader* cs = shader ? shader->GetShader() : nullptr;
        m_DeviceContext->CSSetShader(cs, nullptr, 0);
    }

    void DX_SetRenderTarget(ID3D11DeviceContext1* context, unsigned int numRTs, ID3D11RenderTargetView** rtvs, ID3D11DepthStencilView* dsv, int width, int height)
    {
        const D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
        if (width > 0)
            context->RSSetViewports(1, &viewport);
        context->OMSetRenderTargets(numRTs, rtvs, dsv);
    }

    void GfxDevice::SetRenderTarget(GfxCubemapRenderTarget* cubemapRT, unsigned int face)
    {
        m_RenderTarget = nullptr;
        m_DepthStencil = nullptr;

        ID3D11RenderTargetView* rtv = cubemapRT->GetRTView(face);
        DX_SetRenderTarget(m_DeviceContext, 1, &rtv, nullptr, cubemapRT->GetWidth(), cubemapRT->GetHeight());
    }

    void GfxDevice::SetRenderTarget(GfxRenderTarget* renderTarget)
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

    void GfxDevice::SetDepthStencil(GfxRenderTarget* depthStencil)
    {
        m_DepthStencil = depthStencil;
        SetRenderTarget(m_RenderTarget);
    }

    void GfxDevice::SetStencilRef(unsigned int ref)
    {
        m_StencilRef = ref;
        m_DeviceContext->OMSetDepthStencilState(m_State->GetDepthStencilState(), m_StencilRef);
    }

    void GfxDevice::Dispatch(unsigned int x, unsigned int y, unsigned int z)
    {
        m_DeviceContext->Dispatch(x, y, z);
    }

    void GfxDevice::Draw(unsigned int numVerts)
    {
        m_DeviceContext->Draw(numVerts, 0);
    }

    void GfxDevice::DrawIndexed(unsigned int numIndices)
    {
        m_DeviceContext->DrawIndexed(numIndices, 0, 0);
    }

    void GfxDevice::DrawFullSceen()
    {
        BindVertexBuffer(GfxDefaults::VB_2DQUAD);
        Draw(6);
    }

    void GfxDevice::BeginPass(const std::string& debugName)
    {
#ifdef DEBUG
        std::wstring wDebugName = StringUtil::ToWideString(debugName);
        m_DebugMarkers->BeginEvent(wDebugName.c_str());
#endif
    }

    void GfxDevice::EndPass()
    {
#ifdef DEBUG
        m_DebugMarkers->EndEvent();
#endif
    }

    void GfxDevice::Present()
    {
        m_SwapChain->Present(1, 0);
    }

    bool GfxDevice::CreateDevice()
    {
        ID3D11Device* baseDevice;
        ID3D11DeviceContext* baseDeviceContext;
        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef DEBUG
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

#ifdef DEBUG
    void GfxDevice::InitDebugLayer()
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
#endif // DEBUG

    void GfxDevice::CreateSwapChain()
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
        d3d11SwapChainDesc.Width = Window::Get()->GetWidth();
        d3d11SwapChainDesc.Height = Window::Get()->GetHeight();
        d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        d3d11SwapChainDesc.SampleDesc.Count = 1;
        d3d11SwapChainDesc.SampleDesc.Quality = 0;
        d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        d3d11SwapChainDesc.BufferCount = 2;
        d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        d3d11SwapChainDesc.Flags = 0;

        DX_CALL(dxgiFactory->CreateSwapChainForHwnd(m_Device, Window::Get()->GetHandle(), &d3d11SwapChainDesc, 0, 0, &m_SwapChain));

        dxgiFactory->Release();

        m_FinalRT = GfxRenderTarget::CreateFromSwapChain(m_SwapChain);
    }

    void GfxDevice::InitContext()
    {
        m_DefaultState.Compile();
        BindState(&m_DefaultState);
        SetRenderTarget(m_FinalRT);
        SetDepthStencil(m_FinalRT);

        m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void GfxDevice::InitSamplers()
    {
        D3D11_SAMPLER_DESC samplerDesc = {};
        
        m_Samplers.resize(4);

        // s_PointBorder
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.BorderColor[0] = 0.0f;
        samplerDesc.BorderColor[1] = 0.0f;
        samplerDesc.BorderColor[2] = 0.0f;
        samplerDesc.BorderColor[3] = 1.0f;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        DX_CALL(m_Device->CreateSamplerState(&samplerDesc, &m_Samplers[0]));

        // s_LinearBorder
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.BorderColor[0] = 0.0f;
        samplerDesc.BorderColor[1] = 0.0f;
        samplerDesc.BorderColor[2] = 0.0f;
        samplerDesc.BorderColor[3] = 1.0f;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        DX_CALL(m_Device->CreateSamplerState(&samplerDesc, &m_Samplers[1]));

        // s_LinearClamp
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        DX_CALL(m_Device->CreateSamplerState(&samplerDesc, &m_Samplers[2]));
        
        // s_LinearWrap
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        DX_CALL(m_Device->CreateSamplerState(&samplerDesc, &m_Samplers[3]));

        m_DeviceContext->VSSetSamplers(0, m_Samplers.size(), m_Samplers.data());
        m_DeviceContext->PSSetSamplers(0, m_Samplers.size(), m_Samplers.data());
        m_DeviceContext->GSSetSamplers(0, m_Samplers.size(), m_Samplers.data());
        m_DeviceContext->CSSetSamplers(0, m_Samplers.size(), m_Samplers.data());
    }

    void GfxDevice::ClearPipeline()
    {
        std::vector<ID3D11RenderTargetView*> rtViews(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullptr);
        std::vector<ID3D11ShaderResourceView*> srViews(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullptr);
        std::vector<ID3D11Buffer*> cbViews(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, nullptr);
        std::vector<ID3D11SamplerState*> samplerViews(D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, nullptr);
        const FLOAT blendFactor[] = { 1.0f,1.0f,1.0f,1.0f };

        m_DeviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rtViews.data(), nullptr);
        m_DeviceContext->RSSetState(nullptr);
        m_DeviceContext->OMSetDepthStencilState(nullptr, 0xff);
        m_DeviceContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);

        m_DeviceContext->VSSetShader(nullptr, nullptr, 0);
        m_DeviceContext->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srViews.data());
        m_DeviceContext->VSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, cbViews.data());
        m_DeviceContext->VSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, samplerViews.data());

        m_DeviceContext->GSSetShader(nullptr, nullptr, 0);
        m_DeviceContext->GSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srViews.data());
        m_DeviceContext->GSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, cbViews.data());
        m_DeviceContext->GSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, samplerViews.data());

        m_DeviceContext->PSSetShader(nullptr, nullptr, 0);
        m_DeviceContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srViews.data());
        m_DeviceContext->PSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, cbViews.data());
        m_DeviceContext->PSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, samplerViews.data());

        m_DeviceContext->CSSetShader(nullptr, nullptr, 0);
        m_DeviceContext->CSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srViews.data());
        m_DeviceContext->CSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, cbViews.data());
        m_DeviceContext->CSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, samplerViews.data());
    }

    ///////////////////////////////////////
    //			Shader  		        //
    /////////////////////////////////////

    GfxShader::GfxShader(const std::string& path, bool skipPS)
#ifdef DEBUG
        : m_Path(path)
#endif // DEBUG
    {
#ifdef DEBUG
        if (skipPS) m_PSEntry = "";
#endif // DEBUG
        bool success = CompileShader(path, DEFAULT_VS_ENTRY, skipPS ? "" : DEFAULT_PS_ENTRY);
        ASSERT(success, "Shader compilation failed!");
        m_Initialized = true;
    }

    GfxShader::GfxShader(const std::string& path, const std::string& vsEntry, const std::string& psEntry, bool skipPS)
#ifdef DEBUG
        : m_Path(path),
        m_VSEntry(vsEntry),
        m_PSEntry(skipPS ? "" : psEntry)
#endif // DEBUG
    {
        bool success = CompileShader(path, vsEntry, skipPS ? "" : psEntry);
        ASSERT(success, "Shader compilation failed!");
        m_Initialized = true;
    }

    GfxShader::~GfxShader()
    {
        m_VertexShader->Release();
        if (m_PixelShader)
            m_PixelShader->Release();
        m_InputLayout->Release();
    }

    void GfxShader::Reload()
    {
#ifdef DEBUG
        ShaderFactory* sf = g_Device->GetShaderFactory();
        sf->SetVSEntry(m_VSEntry);
        sf->SetPSEntry(m_PSEntry);
        sf->SetCSEntry("");
        CompiledShader compiledShader = sf->CompileShader(m_Path);

        if (compiledShader.valid)
        {
            m_VertexShader->Release();
            if (m_PixelShader)
                m_PixelShader->Release();
            m_InputLayout->Release();

            m_VertexShader = compiledShader.vs;
            m_PixelShader = compiledShader.ps;
            m_InputLayout = compiledShader.il;
        }
#endif
    }

    bool GfxShader::CompileShader(const std::string& path, const std::string& vsEntry, const std::string psEntry)
    {
        ShaderFactory* sf = g_Device->GetShaderFactory();
        sf->SetVSEntry(vsEntry);
        sf->SetPSEntry(psEntry);
        sf->SetCSEntry("");
        CompiledShader compiledShader = sf->CompileShader(path);
       
        m_VertexShader = compiledShader.vs;
        m_PixelShader = compiledShader.ps;
        m_InputLayout = compiledShader.il;

        return compiledShader.valid;
    }

    ///////////////////////////////////////
    //			ComputeShader           //
    /////////////////////////////////////

    GfxComputeShader::GfxComputeShader(const std::string& path)
#ifdef DEBUG
        : m_Path(path)
#endif // DEBUG
    {
        bool success = CompileShader(path, DEFAULT_ENTRY);
        ASSERT(success, "Compute shader compilation failed!");
        m_Initialized = true;
    }

    GfxComputeShader::GfxComputeShader(const std::string& path, const std::string& entryPoint)
#ifdef DEBUG
        : m_Path(path),
        m_Entry(entryPoint)
#endif // DEBUG
    {
        bool success = CompileShader(path, entryPoint);
        ASSERT(success, "Compute shader compilation failed!");
        m_Initialized = true;
    }

    GfxComputeShader::~GfxComputeShader()
    {
        m_Shader->Release();
    }

    void GfxComputeShader::Reload()
    {
#ifdef DEBUG
        ShaderFactory* sf = g_Device->GetShaderFactory();
        sf->SetVSEntry("");
        sf->SetPSEntry("");
        sf->SetCSEntry(m_Entry);
        CompiledShader compiledShader = sf->CompileShader(m_Path);

        if (compiledShader.valid)
        {
            m_Shader->Release();
            m_Shader = compiledShader.cs;
        }
#endif
    }

    bool GfxComputeShader::CompileShader(const std::string& path, const std::string& entry)
    {
        ShaderFactory* sf = g_Device->GetShaderFactory();
        sf->SetVSEntry("");
        sf->SetPSEntry("");
        sf->SetCSEntry(entry);
        CompiledShader compiledShader = sf->CompileShader(path);

        m_Shader = compiledShader.cs;
        return compiledShader.valid;
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

    ///////////////////////////////////////
    //			Scoped operations		//
    /////////////////////////////////////
    
    BeginRenderPassScoped::BeginRenderPassScoped(const std::string& debugName)
    {
        g_Device->BeginPass(debugName);
    }

    BeginRenderPassScoped::~BeginRenderPassScoped()
    {
        g_Device->EndPass();
    }

    DeviceStateScoped::DeviceStateScoped(GfxDeviceState* state):
        m_LastState(g_Device->GetState())
    {
        g_Device->BindState(state);
    }

    DeviceStateScoped::~DeviceStateScoped()
    {
        g_Device->BindState(m_LastState);
    }

    RenderTargetScoped::RenderTargetScoped(GfxRenderTarget* rt, GfxRenderTarget* ds):
        m_LastRT(g_Device->GetRenderTarget()),
        m_LastDS(g_Device->GetDepthStencil())
    {
        // First we must detach DS, in case that new render target size doesn't match old DS size
        g_Device->SetDepthStencil(nullptr);

        g_Device->SetRenderTarget(rt);
        g_Device->SetDepthStencil(ds);
    }

    RenderTargetScoped::~RenderTargetScoped()
    {
        g_Device->SetDepthStencil(nullptr);

        g_Device->SetRenderTarget(m_LastRT);
        g_Device->SetDepthStencil(m_LastDS);
    }
}


