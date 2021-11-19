#include "GfxDevice.h"

#include <d3d11_1.h>

#ifdef DEBUG
#include <dxgidebug.h>
#include "util/StringUtil.h"
#endif

#include "Common.h"
#include "core/Window.h"
#include "gui/GUI.h"
#include "gfx/GfxTexture.h"
#include "gfx/GfxShader.h"

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

        D3D11_BLEND_OP GetDXBlendOp(BlendOp blendOp)
        {
            switch (blendOp)
            {
            case BlendOp::Add: return D3D11_BLEND_OP_ADD;
            case BlendOp::Substract: return D3D11_BLEND_OP_SUBTRACT;
            case BlendOp::SubstractInv: return D3D11_BLEND_OP_REV_SUBTRACT;
            case BlendOp::Min: return D3D11_BLEND_OP_MIN;
            case BlendOp::Max: return D3D11_BLEND_OP_MAX;
            default: NOT_IMPLEMENTED;
            }
            return D3D11_BLEND_OP_ADD;
        }

        D3D11_BLEND GetDXBlend(Blend blend)
        {
            switch (blend)
            {
            case Blend::Zero: return D3D11_BLEND_ZERO;
            case Blend::One: return D3D11_BLEND_ONE;
            case Blend::SrcColor: return D3D11_BLEND_SRC_COLOR;
            case Blend::SrcColorInv: return D3D11_BLEND_INV_SRC_COLOR;
            case Blend::SrcAlpha: return D3D11_BLEND_SRC_ALPHA;
            case Blend::SrcAlphaInv: return D3D11_BLEND_INV_SRC_ALPHA;
            case Blend::SrcAlphaSat: return D3D11_BLEND_SRC_ALPHA_SAT;
            case Blend::Src1Color: return D3D11_BLEND_SRC1_COLOR;
            case Blend::Src1ColorInv: return D3D11_BLEND_INV_SRC1_COLOR;
            case Blend::Src1Alpha: return D3D11_BLEND_SRC1_ALPHA;
            case Blend::Src1AlphaInv: return D3D11_BLEND_INV_SRC1_ALPHA;
            case Blend::DestColor: return D3D11_BLEND_DEST_COLOR;
            case Blend::DestColorInv: return D3D11_BLEND_INV_DEST_COLOR;
            case Blend::DestAlpha: return D3D11_BLEND_DEST_ALPHA;
            case Blend::DestAlphaInv: return D3D11_BLEND_INV_DEST_ALPHA;
            case Blend::BlendFactor: return D3D11_BLEND_BLEND_FACTOR;
            case Blend::BlendFactorInv: return D3D11_BLEND_INV_BLEND_FACTOR;
            default: NOT_IMPLEMENTED;
            }
            return D3D11_BLEND_ZERO;
        }
    }

    ///////////////////////////////////////
    //			Defaults		        //
    /////////////////////////////////////

    namespace GfxDefaults
    {
        GfxVertexBuffer<Data::VB_CUBE_TYPE>* VB_CUBE = nullptr;
        GfxVertexBuffer<Data::VB_QUAD2D_TYPE>* VB_2DQUAD = nullptr;
        GfxVertexBuffer<Data::VB_QUAD_TYPE>* VB_QUAD = nullptr;
        
        GfxTexture2D* TEX2D_WHITE = nullptr;
        GfxTexture2D* TEX2D_BLACK = nullptr;

        namespace
        {
            GfxTexture2D* CreateColorTexture(ColorUNORM color)
            {
                GfxTexture2D* texture = new GfxTexture2D(1, 1);
                texture->Upload(&color);
                return texture;
            }
        }

        void InitDefaults()
        {
            VB_CUBE = new GfxVertexBuffer<Data::VB_CUBE_TYPE>((void*) Data::VB_CUBE_DATA, Data::VB_CUBE_SIZE);
            VB_2DQUAD = new GfxVertexBuffer<Data::VB_QUAD2D_TYPE>((void*) Data::VB_QUAD2D_DATA, Data::VB_QUAD2D_SIZE);
            VB_QUAD = new GfxVertexBuffer<Data::VB_QUAD_TYPE>((void*) Data::VB_QUAD_DATA, Data::VB_QUAD_SIZE);

            TEX2D_WHITE = CreateColorTexture({ 255, 255, 255, 255 });
            TEX2D_BLACK = CreateColorTexture({ 0, 0, 0, 255 });
        }

        void DestroyDefaults()
        {
            SAFE_DELETE(VB_CUBE);
            SAFE_DELETE(VB_2DQUAD);
            SAFE_DELETE(VB_QUAD);

            SAFE_DELETE(TEX2D_WHITE);
            SAFE_DELETE(TEX2D_BLACK);
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
        rDesc.FillMode = m_WireframeModeEnabled ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
        rDesc.CullMode = m_BackfaceCullingEnabled ? D3D11_CULL_BACK : D3D11_CULL_NONE;
        rDesc.FrontCounterClockwise = true;
        rDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
        rDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
        rDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rDesc.DepthClipEnable = true;
        rDesc.ScissorEnable = false;
        rDesc.MultisampleEnable = m_MultisamplingEnabled;
        rDesc.AntialiasedLineEnable = false;
        rDesc.ForcedSampleCount = 0;
        DX_CALL(d->CreateRasterizerState1(&rDesc, &m_RasterizerState));

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
        rtbDesc.BlendOp = GetDXBlendOp(m_BlendOp);
        rtbDesc.BlendOpAlpha = GetDXBlendOp(m_BlendAlphaOp);
        rtbDesc.SrcBlend = GetDXBlend(m_SourceColorBlend);
        rtbDesc.DestBlend = GetDXBlend(m_DestColorBlend);
        rtbDesc.SrcBlendAlpha = GetDXBlend(m_SourceAlphaBlend);
        rtbDesc.DestBlendAlpha = GetDXBlend(m_DestAlphaBlend);
        rtbDesc.LogicOpEnable = false;
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
    //			Input assembler         //
    /////////////////////////////////////

    static inline DXGI_FORMAT IndexStrideToDXGIFormat(unsigned int indexStride)
    {
        switch (indexStride)
        {
        case 0: return DXGI_FORMAT_UNKNOWN;
        case 2: return DXGI_FORMAT_R16_UINT;
        case 4: return DXGI_FORMAT_R32_UINT;
        default: NOT_IMPLEMENTED;
        }
        return DXGI_FORMAT_UNKNOWN;
    }

    static inline D3D11_PRIMITIVE_TOPOLOGY ToDXTopology(PrimitiveTopology topology)
    {
        switch (topology)
        {
        case PrimitiveTopology::Points:             return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        case PrimitiveTopology::Lines:              return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        case PrimitiveTopology::LineStrip:          return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case PrimitiveTopology::Triangles:          return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case PrimitiveTopology::TriangleStrip:      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case PrimitiveTopology::LinesAdj:           return D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
        case PrimitiveTopology::LineStripAdj:       return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
        case PrimitiveTopology::TrianglesAdj:       return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
        case PrimitiveTopology::TriangleStripAdj:   return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
        default: NOT_IMPLEMENTED;
        }
        return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }

    void GfxInputAssembler::PrepareForDraw(GfxShader* shader)
    {
        static ID3D11Buffer* NULL_BUFFER[] = { nullptr };
        static unsigned int NULL_VALUE[] = { 0 };

        if (m_Dirty)
        {
            ID3D11DeviceContext1* context = g_Device->GetDeviceContext();

            // Set primitive topology
            context->IASetPrimitiveTopology(ToDXTopology(m_PrimitiveTopology));

            // Bind vertex buffers
            unsigned int numBuffers = m_VBResources.size();
            ID3D11Buffer** buffers = numBuffers ? m_VBResources.data() : NULL_BUFFER;
            unsigned int* strides = numBuffers ? m_VBStrides.data() : NULL_VALUE;
            unsigned int* offsets = numBuffers ? m_VBOffsets.data() : NULL_VALUE;
            context->IASetVertexBuffers(0, numBuffers, buffers, strides, offsets);

            // Bind index buffers
            if (m_IBResource)
                context->IASetIndexBuffer(m_IBResource, IndexStrideToDXGIFormat(m_IBStride), m_IBOffset);
            else
                context->IASetIndexBuffer(nullptr, IndexStrideToDXGIFormat(0), 0);

            // Bind input layout
            if (shader)
            {
                ID3D11InputLayout* inputLayout = m_VBResources.size() > 1 ? shader->GetMIL() : shader->GetIL();
                ASSERT(inputLayout, "ERROR: InputLayout is null. Please bind InstanceBuffer if you are using per instance inputs in shader.");
                context->IASetInputLayout(inputLayout);
            }
            else
            {
                context->IASetInputLayout(nullptr);
            }

            m_Dirty = false;
        }
    }

    ///////////////////////////////////////
    //			Context                 //
    /////////////////////////////////////

    GfxDeferredContext::GfxDeferredContext()
    {
        ID3D11Device1* d = g_Device->GetDevice();
        for (size_t i = 0; i < NUM_HANDLES; i++) DX_CALL(d->CreateDeferredContext1(0, &m_Handle[i]));
    }

    GfxDeferredContext::~GfxDeferredContext()
    {
        for(size_t i=0; i < NUM_HANDLES; i++) m_Handle[i]->Release();
    }

    void GfxDeferredContext::Submit()
    {
        unsigned int next = (m_Current + 1) % NUM_HANDLES;

        ID3D11CommandList* commandList = nullptr;
        DX_CALL(m_Handle[next]->FinishCommandList(FALSE, &commandList));
        g_Device->GetDeviceContext()->ExecuteCommandList(commandList, TRUE);
        commandList->Release();

        m_Current = next;
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

        m_GraphicsThread = CURRENT_THREAD;

#ifdef DEBUG
        InitDebugLayer();
#endif

        CreateSwapChain();

        InitContext();
        InitSamplers();

        m_Initialized = true;

        GfxDefaults::InitDefaults();

        g_GUI = new GUI(Window::Get()->GetHandle(), m_Device, m_DeviceContext);
    }

    GfxDevice::~GfxDevice()
    {
        GfxDefaults::DestroyDefaults();
        delete m_FinalRT;
        for (GfxSampler* sampler : m_Samplers) delete sampler;
        for (auto& it : m_ThreadContexts) delete it.second;
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
                m_DeviceContext->ClearRenderTargetView(m_RenderTarget->GetRTV(i), clearColor);
            }
        }

        if (m_DepthStencil)
        {
            m_DeviceContext->ClearDepthStencilView(m_DepthStencil->GetDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
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

    void GfxDevice::BindTexture2D(unsigned int shaderStage, GfxTexture2D* texture, unsigned int binding)
    {
        DX_BindTexture(m_DeviceContext, shaderStage, texture->GetSRV(), binding);
    }

    void GfxDevice::BindTextureArray2D(unsigned int shaderStage, GfxTextureArray2D* textureArray, unsigned int binding)
    {
        DX_BindTexture(m_DeviceContext, shaderStage, textureArray->GetSRV(), binding);
    }

    void GfxDevice::BindCubemap(unsigned int shaderStage, GfxCubemap* cubemap, unsigned int binding)
    {
        DX_BindTexture(m_DeviceContext, shaderStage, cubemap->GetSRV(), binding);
    }

    void GfxDevice::UnbindTexture(unsigned int shaderStage, unsigned int binding)
    {
        DX_UnbindTexture(m_DeviceContext, shaderStage, binding);
    }

    void GfxDevice::BindSampler(unsigned int shaderStage, GfxSampler* sampler, unsigned int binding)
    {
        ASSERT(binding < m_MaxCustomSamplers, "[GfxDevice::BindSampler] " + std::to_string(binding) + " is out of the limit, maximum binding is " + std::to_string(m_MaxCustomSamplers-1));

        ID3D11SamplerState* samplerState = sampler ? sampler->GetSampler() : nullptr;

        if (shaderStage & VS)
            m_DeviceContext->VSSetSamplers(binding, 1, &samplerState);

        if (shaderStage & GS)
            m_DeviceContext->GSSetSamplers(binding, 1, &samplerState);

        if (shaderStage & PS)
            m_DeviceContext->PSSetSamplers(binding, 1, &samplerState);

        if (shaderStage & CS)
            m_DeviceContext->CSSetSamplers(binding, 1, &samplerState);
    }

    void GfxDevice::BindShader(GfxShader* shader)
    {
        m_Shader = shader;

        if (shader)
        {
            m_DeviceContext->VSSetShader(shader->GetVS(), nullptr, 0);
            m_DeviceContext->PSSetShader(shader->GetPS(), nullptr, 0);
            m_DeviceContext->DSSetShader(shader->GetDS(), nullptr, 0);
            m_DeviceContext->HSSetShader(shader->GetHS(), nullptr, 0);
            m_DeviceContext->GSSetShader(shader->GetGS(), nullptr, 0);
            m_DeviceContext->CSSetShader(shader->GetCS(), nullptr, 0);
        }
        else
        {
            m_DeviceContext->VSSetShader(nullptr, nullptr, 0);
            m_DeviceContext->PSSetShader(nullptr, nullptr, 0);
            m_DeviceContext->DSSetShader(nullptr, nullptr, 0);
            m_DeviceContext->HSSetShader(nullptr, nullptr, 0);
            m_DeviceContext->GSSetShader(nullptr, nullptr, 0);
            m_DeviceContext->CSSetShader(nullptr, nullptr, 0);
        }
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
    
        ID3D11RenderTargetView* rtv = cubemapRT->GetRTV(face);
        DX_SetRenderTarget(m_DeviceContext, 1, &rtv, nullptr, cubemapRT->GetWidth(), cubemapRT->GetHeight());
    }

    void GfxDevice::SetRenderTarget(GfxRenderTarget* renderTarget)
    {
        m_RenderTarget = renderTarget;

        unsigned int numRTs = m_RenderTarget ? m_RenderTarget->GetNumRTs() : 0;
        ID3D11RenderTargetView** rtvs = m_RenderTarget ? m_RenderTarget->GetRTVs() : nullptr;
        ID3D11DepthStencilView* dsv = m_DepthStencil ? m_DepthStencil->GetDSV() : nullptr;
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
        m_InputAssember.PrepareForDraw(m_Shader);
        m_DeviceContext->Draw(numVerts, 0);
    }

    void GfxDevice::DrawIndexed(unsigned int numIndices)
    {
        m_InputAssember.PrepareForDraw(m_Shader);
        m_DeviceContext->DrawIndexed(numIndices, 0, 0);
    }

    void GfxDevice::DrawInstanced(unsigned int numVerts, unsigned int numInstances)
    {
        m_InputAssember.PrepareForDraw(m_Shader);
        m_DeviceContext->DrawInstanced(numVerts, numInstances, 0, 0);
    }

    void GfxDevice::DrawIndexedInstanced(unsigned int numIndices, unsigned int numInstances)
    {
        m_InputAssember.PrepareForDraw(m_Shader);
        m_DeviceContext->DrawIndexedInstanced(numIndices, numInstances, 0, 0, 0);
    }

    void GfxDevice::DrawFullSceen()
    {
        m_InputAssember.PrepareForDraw(m_Shader);
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

    void GfxDevice::EndFrame()
    {
        // Submit contexts
        for (auto& it : m_ThreadContexts) it.second->Submit();

        // Delete contexts that requested it
        auto& threadContexts = m_ThreadContexts;
        const auto deleteContexts = [&threadContexts](ThreadID threadID) {
            delete threadContexts[threadID];
            threadContexts.erase(threadID);
        };
        
        m_ContextsToDelete.Lock();
        m_ContextsToDelete.ForEach(deleteContexts);
        m_ContextsToDelete.Clear();
        m_ContextsToDelete.Unlock();

        // Present to screen
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
    }

    void GfxDevice::InitSamplers()
    {
        D3D11_SAMPLER_DESC samplerDesc = {};
        
        m_Samplers.resize(4);
        m_Samplers[0] = new GfxSampler(SamplerFilter::Point, SamplerMode::Border);
        m_Samplers[1] = new GfxSampler(SamplerFilter::Linear, SamplerMode::Border);
        m_Samplers[2] = new GfxSampler(SamplerFilter::Linear, SamplerMode::Clamp);
        m_Samplers[3] = new GfxSampler(SamplerFilter::Linear, SamplerMode::Wrap);

        m_MaxCustomSamplers = 16 - m_Samplers.size();

        ID3D11SamplerState** samplers = (ID3D11SamplerState**) malloc(m_Samplers.size() * sizeof(ID3D11SamplerState*));
        for (size_t i = 0; i < m_Samplers.size(); i++)
        {
            samplers[i] = m_Samplers[i]->GetSampler();
        }

        m_DeviceContext->VSSetSamplers(m_MaxCustomSamplers, m_Samplers.size(), samplers);
        m_DeviceContext->PSSetSamplers(m_MaxCustomSamplers, m_Samplers.size(), samplers);
        m_DeviceContext->GSSetSamplers(m_MaxCustomSamplers, m_Samplers.size(), samplers);
        m_DeviceContext->CSSetSamplers(m_MaxCustomSamplers, m_Samplers.size(), samplers);
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
}


