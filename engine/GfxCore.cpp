#include "GfxCore.h"

#include <d3d11_1.h>
#include <d3dcompiler.h>

#include "core/Core.h"

#define DX_CALL(X) {HRESULT hr = X; ASSERT(SUCCEEDED(hr), "DX ERROR " __FILE__);}
#define SAFE_RELEASE(X) if(X) { X->Release(); X = nullptr; }

#ifdef DEBUG_GFX
#include <dxgidebug.h>
#include "util/StringUtil.h"
#endif

#include "core/Core.h"
#include "core/Window.h"

namespace GP
{
    static GfxVertexBuffer* QUAD_BUFFER = nullptr;

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

        DXGI_FORMAT ToDXGIFormat(ShaderInputFormat format)
        {
            switch (format)
            {
            case ShaderInputFormat::Float:
                return DXGI_FORMAT_R32_FLOAT;
            case ShaderInputFormat::Float2:
                return DXGI_FORMAT_R32G32_FLOAT;
            case ShaderInputFormat::Float3:
                return DXGI_FORMAT_R32G32B32_FLOAT;
            case ShaderInputFormat::Float4:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            default:
                NOT_IMPLEMENTED;
            }
            return DXGI_FORMAT_UNKNOWN;
        }

        inline bool CompileShader(ID3D11Device1* device, const ShaderDesc& desc, ID3D11VertexShader*& vs, ID3D11PixelShader*& ps, ID3D11InputLayout*& il)
        {
            std::wstring wsPath = StringUtil::ToWideString(desc.path);

            // Create Vertex Shader
            ID3DBlob* vsBlob;
            {
                ID3DBlob* shaderCompileErrorsBlob;
                HRESULT hResult = D3DCompileFromFile(wsPath.c_str(), nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, &shaderCompileErrorsBlob);
                if (FAILED(hResult))
                {
                    const char* errorString = NULL;
                    if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                        errorString = "Could not compile shader; file not found";
                    else if (shaderCompileErrorsBlob) {
                        errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
                        shaderCompileErrorsBlob->Release();
                    }
                    MessageBoxA(0, errorString, "VS Shader Compiler Error", MB_ICONERROR | MB_OK);
                    return false;
                }

                hResult = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs);
                ASSERT(SUCCEEDED(hResult), "VS Shader create fail");
            }

            // Create Pixel Shader
            if (!desc.skipPS)
            {
                ID3DBlob* psBlob;
                ID3DBlob* shaderCompileErrorsBlob;
                HRESULT hResult = D3DCompileFromFile(wsPath.c_str(), nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorsBlob);
                if (FAILED(hResult))
                {
                    const char* errorString = NULL;
                    if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                        errorString = "Could not compile shader; file not found";
                    else if (shaderCompileErrorsBlob) {
                        errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
                        shaderCompileErrorsBlob->Release();
                    }
                    MessageBoxA(0, errorString, "PS Shader Compiler Error", MB_ICONERROR | MB_OK);
                    return false;
                }

                hResult = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps);
                ASSERT(SUCCEEDED(hResult), "PS Shader create fail");
                psBlob->Release();
            }

            // Create Input Layout
            std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements(desc.inputs.size());
            size_t inputIndex = 0;
            for (ShaderInput input : desc.inputs)
            {
                D3D11_INPUT_ELEMENT_DESC& elementDesc = inputElements[inputIndex];
                elementDesc.SemanticName = input.semanticName;
                elementDesc.SemanticIndex = 0;
                elementDesc.Format = ToDXGIFormat(input.format);
                elementDesc.InputSlot = 0;
                elementDesc.AlignedByteOffset = inputIndex == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
                elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                elementDesc.InstanceDataStepRate = 0;

                inputIndex++;
            }

            HRESULT hResult = device->CreateInputLayout(inputElements.data(), inputElements.size(), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &il);
            vsBlob->Release();
            return SUCCEEDED(hResult);
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
    //			DeviceState		        //
    /////////////////////////////////////

    GfxDeviceState::~GfxDeviceState()
    {
        SAFE_RELEASE(m_RasterizerState);
        SAFE_RELEASE(m_DepthStencilState);
        SAFE_RELEASE(m_BlendState);
    }

    void GfxDeviceState::Compile(GfxDevice* device)
    {
        ASSERT(!m_Compiled, "[GfxDeviceState] Already compiled this state!");

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

    ///////////////////////////////////////
    //			Device  		        //
    /////////////////////////////////////

    GfxDevice::GfxDevice(Window* window)
    {
        if (!CreateDevice()) return;

#ifdef DEBUG_GFX
        InitDebugLayer();
#endif

        CreateSwapChain(window);

        InitContext(window);
        InitSamplers();

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

        QUAD_BUFFER = new GfxVertexBuffer(this, quadData);
    }

    GfxDevice::~GfxDevice()
    {
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

    void GfxDevice::BindConstantBuffer(ShaderStage stage, ID3D11Buffer* constantBuffer, unsigned int binding)
    {
        if (stage & VS)
            m_DeviceContext->VSSetConstantBuffers(binding, 1, &constantBuffer);

        if (stage & PS)
            m_DeviceContext->PSSetConstantBuffers(binding, 1, &constantBuffer);
    }

    void GfxDevice::BindStructuredBuffer(ShaderStage stage, ID3D11ShaderResourceView* structuredBufferSrv, unsigned int binding)
    {
        if (stage & VS)
            m_DeviceContext->VSSetShaderResources(binding, 1, &structuredBufferSrv);

        if (stage & PS)
            m_DeviceContext->PSSetShaderResources(binding, 1, &structuredBufferSrv);
    }

    void GfxDevice::BindIndexBuffer(GfxIndexBuffer* indexBuffer)
    {
        m_DeviceContext->IASetIndexBuffer(indexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, indexBuffer->GetOffset());
    }

    void GfxDevice::BindVertexBuffer(GfxVertexBuffer* vertexBuffer)
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

    void GfxDevice::BindTexture(ShaderStage stage, GfxTexture* texture, unsigned int binding)
    {
        DX_BindTexture(m_DeviceContext, stage, texture->GetTextureView(), binding);
    }

    void GfxDevice::BindTexture(ShaderStage stage, GfxRenderTarget* renderTarget, unsigned int binding, unsigned int texIndex)
    {
        ID3D11ShaderResourceView* srv = texIndex == -1 ? renderTarget->GetDSSRView() : renderTarget->GetSRView(texIndex);
        DX_BindTexture(m_DeviceContext, stage, srv, binding);
    }

    void GfxDevice::BindTexture(ShaderStage stage, GfxCubemapRenderTarget* cubemapRT, unsigned int binding)
    {
        DX_BindTexture(m_DeviceContext, stage, cubemapRT->GetSRView(), binding);
    }

    void GfxDevice::UnbindTexture(ShaderStage stage, unsigned int binding)
    {
        DX_UnbindTexture(m_DeviceContext, stage, binding);
    }

    void GfxDevice::BindShader(GfxShader* shader)
    {
        m_DeviceContext->IASetInputLayout(shader->GetInputLayout());
        m_DeviceContext->VSSetShader(shader->GetVertexShader(), nullptr, 0);
        m_DeviceContext->PSSetShader(shader->GetPixelShader(), nullptr, 0);
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
        BindVertexBuffer(QUAD_BUFFER);
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

    void GfxDevice::CreateSwapChain(Window* window)
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

        m_FinalRT = GfxRenderTarget::CreateFromSwapChain(this, m_SwapChain);
    }

    void GfxDevice::InitContext(Window* window)
    {
        m_DefaultState.Compile(this);
        BindState(&m_DefaultState);
        SetRenderTarget(m_FinalRT);
        SetDepthStencil(m_FinalRT);

        m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void GfxDevice::InitSamplers()
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

    ///////////////////////////////////////
    //			Shader  		        //
    /////////////////////////////////////

    namespace
    {

    }
    
    GfxShader::GfxShader(GfxDevice* device, const ShaderDesc& desc)
    {
#ifdef DEBUG
        m_Desc = desc;
#endif

        bool success = CompileShader(device->GetDevice(), desc, m_VertexShader, m_PixelShader, m_InputLayout);
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

    void GfxShader::Reload(GfxDevice* device)
    {
#ifdef DEBUG
        ID3D11VertexShader* vs;
        ID3D11PixelShader* ps = nullptr;
        ID3D11InputLayout* il;

        if (CompileShader(device->GetDevice(), m_Desc, vs, ps, il))
        {
            m_VertexShader->Release();
            if (m_PixelShader)
                m_PixelShader->Release();
            m_InputLayout->Release();

            m_VertexShader = vs;
            m_PixelShader = ps;
            m_InputLayout = il;
        }
#endif
    }

    ///////////////////////////////////////////
    /// Buffer Functions                 /////
    /////////////////////////////////////////

    ID3D11Buffer* CreateConstantBuffer(GfxDevice* device, unsigned int bufferSize)
    {
        ID3D11Buffer* buffer = nullptr;

        D3D11_BUFFER_DESC bufferDesc = {};
        // ByteWidth must be a multiple of 16, per the docs
        bufferDesc.ByteWidth = bufferSize + 0xf & 0xfffffff0;
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        DX_CALL(device->GetDevice()->CreateBuffer(&bufferDesc, nullptr, &buffer));

        return buffer;
    }

    ID3D11Buffer* CreateStructuredBuffer(GfxDevice* device, unsigned int elementSize, unsigned int numElements)
    {
        ID3D11Buffer* buffer = nullptr;

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = elementSize * numElements;
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride = elementSize;

        DX_CALL(device->GetDevice()->CreateBuffer(&bufferDesc, NULL, &buffer));

        return buffer;
    }

    ID3D11ShaderResourceView* CreateStructuredBufferView(GfxDevice* device, ID3D11Buffer* structuredBuffer, unsigned int numElements)
    {
        ID3D11ShaderResourceView* srv;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.NumElements = numElements;

        DX_CALL(device->GetDevice()->CreateShaderResourceView(structuredBuffer, &srvDesc, &srv));

        return srv;
    }

    void ReleaseBuffer(ID3D11Buffer* buffer)
    {
        buffer->Release();
    }

    void ReleaseSRV(ID3D11ShaderResourceView* srv)
    {
        srv->Release();
    }

    void UploadToBuffer(GfxDevice* device, ID3D11Buffer* buffer, const void* data, unsigned int numBytes, unsigned int offset)
    {
        ID3D11DeviceContext1* deviceContext = device->GetDeviceContext();

        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        DX_CALL(deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource));
        byte* bufferPtr = (byte*)mappedSubresource.pData;
        memcpy(bufferPtr + offset, data, numBytes);
        deviceContext->Unmap(buffer, 0);
    }

    ///////////////////////////////////////////
    /// Vertex buffer                    /////
    /////////////////////////////////////////

    GfxVertexBuffer::GfxVertexBuffer(GfxDevice* device, const VertexBufferData& data)
    {
        m_Stride = data.stride;
        m_NumVerts = data.numBytes / data.stride;
        m_Offset = 0;

        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.ByteWidth = data.numBytes;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubresourceData = { data.pData };

        DX_CALL(device->GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &m_Buffer));
    }

    GfxVertexBuffer::~GfxVertexBuffer()
    {
        if (m_BufferOwner)
            m_Buffer->Release();
    }

    ///////////////////////////////////////////
    /// Index buffer                     /////
    /////////////////////////////////////////

    GfxIndexBuffer::GfxIndexBuffer(GfxDevice* device, unsigned int* pIndices, unsigned int numIndices):
        m_NumIndices(numIndices)
    {
        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.ByteWidth = sizeof(unsigned int) * numIndices;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubresourceData = { pIndices };

        DX_CALL(device->GetDevice()->CreateBuffer(&indexBufferDesc, &vertexSubresourceData, &m_Buffer));
    }

    GfxIndexBuffer::~GfxIndexBuffer()
    {
        if (m_BufferOwner)
            m_Buffer->Release();
    }

    ///////////////////////////////////////////
    /// Texture                          /////
    /////////////////////////////////////////

    GfxTexture::GfxTexture(GfxDevice* device, const TextureDesc& desc) :
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

        DX_CALL(device->GetDevice()->CreateTexture2D(&textureDesc, textureSubresourceData, &m_Texture));
        DX_CALL(device->GetDevice()->CreateShaderResourceView(m_Texture, nullptr, &m_TextureView));
    }

    GfxTexture::~GfxTexture()
    {
        m_Texture->Release();
        m_TextureView->Release();
    }

    ///////////////////////////////////////////
    /// RenderTarget                     /////
    /////////////////////////////////////////

    GfxRenderTarget* GfxRenderTarget::CreateFromSwapChain(GfxDevice* device, IDXGISwapChain1* swapchain)
    {
        GfxRenderTarget* rt = new GfxRenderTarget();
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

    GfxRenderTarget::GfxRenderTarget(GfxDevice* device, const RenderTargetDesc& desc)
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

    GfxCubemapRenderTarget::GfxCubemapRenderTarget(GfxDevice* device, const RenderTargetDesc& desc) :
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

    GfxCubemapRenderTarget::~GfxCubemapRenderTarget()
    {
        SAFE_RELEASE(m_TextureMap);
        m_SRView->Release();
        for (size_t i = 0; i < 6; i++) m_RTViews[i]->Release();
    }
}


