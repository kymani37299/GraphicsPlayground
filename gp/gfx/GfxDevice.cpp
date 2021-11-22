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

    void GfxInputAssembler::PrepareForDraw(GfxShader* shader, ID3D11DeviceContext1* context)
    {
        static ID3D11Buffer* NULL_BUFFER[] = { nullptr };
        static unsigned int NULL_VALUE[] = { 0 };

        if (m_Dirty)
        {
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

    namespace
    {
        inline void BindUAV(ID3D11DeviceContext1* context, unsigned int shaderStage, ID3D11UnorderedAccessView* uav, unsigned int binding)
        {
            ASSERT(shaderStage == CS, "[NOT_SUPPORTED] Trying to bind RW resource to stage that isn't compute shader.");
            context->CSSetUnorderedAccessViews(binding, 1, &uav, nullptr);
        }

        inline void BindSRV(ID3D11DeviceContext1* context, unsigned int shaderStage, ID3D11ShaderResourceView* srv, unsigned int binding)
        {
            if (shaderStage & VS)
                context->VSSetShaderResources(binding, 1, &srv);

            if (shaderStage & GS)
                context->GSSetShaderResources(binding, 1, &srv);

            if (shaderStage & PS)
                context->PSSetShaderResources(binding, 1, &srv);

            if (shaderStage & HS)
                context->HSSetShaderResources(binding, 1, &srv);

            if (shaderStage & DS)
                context->DSSetShaderResources(binding, 1, &srv);

            if (shaderStage & CS)
                context->CSSetShaderResources(binding, 1, &srv);
        }

        void BindCB(ID3D11DeviceContext1* context, unsigned int shaderStage, ID3D11Buffer* buffer, unsigned int binding)
        {
            if (shaderStage & VS)
                context->VSSetConstantBuffers(binding, 1, &buffer);

            if (shaderStage & GS)
                context->GSSetConstantBuffers(binding, 1, &buffer);

            if (shaderStage & PS)
                context->PSSetConstantBuffers(binding, 1, &buffer);

            if (shaderStage & CS)
                context->CSSetConstantBuffers(binding, 1, &buffer);

            if (shaderStage & HS)
                context->HSSetConstantBuffers(binding, 1, &buffer);

            if (shaderStage & DS)
                context->DSSetConstantBuffers(binding, 1, &buffer);
        }

        void BindRT(ID3D11DeviceContext1* context, unsigned int numRTs, ID3D11RenderTargetView** rtvs, ID3D11DepthStencilView* dsv, int width, int height)
        {
            const D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
            if (width > 0) context->RSSetViewports(1, &viewport);
            context->OMSetRenderTargets(numRTs, rtvs, dsv);
        }
    }

    GfxContext::GfxContext():
        m_Deferred(true)
    {
        ID3D11Device1* d = g_Device->GetDevice();
        for (size_t i = 0; i < NUM_DEFERRED_HANDLES; i++) DX_CALL(d->CreateDeferredContext1(0, &m_Handles[i]));
        SwitchCurrentHandle(0);
    }

    GfxContext::GfxContext(ID3D11DeviceContext1* context):
        m_Deferred(false)
    {
        ASSERT(context, "[GfxContext] Trying to initialize immediate context with null");
        m_Handles[0] = context;
        SwitchCurrentHandle(0);

#ifdef DEBUG
        InitDebugLayer();
#endif
    }

    GfxContext::~GfxContext()
    {
        size_t numHandles = m_Deferred ? NUM_DEFERRED_HANDLES : 1;
        for (size_t i = 0; i < numHandles; i++) m_Handles[i]->Release();
    }

    void GfxContext::Clear(const Vec4& color)
    {
        const FLOAT clearColor[4] = { color.x, color.y, color.z, color.w };

        if (m_RenderTarget)
        {
            for (size_t i = 0; i < m_RenderTarget->GetNumRTs(); i++)
            {
                m_Handles[m_Current]->ClearRenderTargetView(m_RenderTarget->GetRTV(i), clearColor);
            }
        }

        if (m_DepthStencil)
        {
            m_Handles[m_Current]->ClearDepthStencilView(m_DepthStencil->GetDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        }
    }

    void GfxContext::BindConstantBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
    {
        BindCB(m_Handles[m_Current], shaderStage, GetDeviceHandle(gfxBuffer), binding);
    }

    void GfxContext::BindStructuredBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
    {
        BindSRV(m_Handles[m_Current], shaderStage, GetDeviceSRV(gfxBuffer), binding);
    }

    void GfxContext::BindRWStructuredBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
    {
        BindUAV(m_Handles[m_Current], shaderStage, GetDeviceUAV(gfxBuffer), binding);
    }

    void GfxContext::BindTexture2D(unsigned int shaderStage, GfxTexture2D* texture, unsigned int binding)
    {
        BindSRV(m_Handles[m_Current], shaderStage, GetDeviceSRV(texture), binding);
    }

    void GfxContext::BindTexture3D(unsigned int shaderStage, GfxTexture3D* texture, unsigned int binding)
    {
        BindSRV(m_Handles[m_Current], shaderStage, GetDeviceSRV(texture), binding);
    }

    void GfxContext::BindRWTexture3D(unsigned int shaderStage, GfxRWTexture3D* texture, unsigned int binding)
    {
        BindUAV(m_Handles[m_Current], shaderStage, GetDeviceUAV(texture), binding);
    }

    void GfxContext::BindTextureArray2D(unsigned int shaderStage, GfxTextureArray2D* textureArray, unsigned int binding)
    {
        BindSRV(m_Handles[m_Current], shaderStage, GetDeviceSRV(textureArray), binding);
    }

    void GfxContext::BindCubemap(unsigned int shaderStage, GfxCubemap* cubemap, unsigned int binding)
    {
        BindSRV(m_Handles[m_Current], shaderStage, GetDeviceSRV(cubemap), binding);
    }

    void GfxContext::UnbindTexture(unsigned int shaderStage, unsigned int binding)
    {
        BindSRV(m_Handles[m_Current], shaderStage, nullptr, binding);
    }

    void GfxContext::BindSampler(unsigned int shaderStage, GfxSampler* sampler, unsigned int binding)
    {
        ASSERT(binding < g_Device->GetMaxCustomSamplers(), "[GfxDevice::BindSampler] " + std::to_string(binding) + " is out of the limit, maximum binding is " + std::to_string(g_Device->GetMaxCustomSamplers() - 1));

        ID3D11SamplerState* samplerState = sampler ? sampler->GetSampler() : nullptr;

        if (shaderStage & VS)
            m_Handles[m_Current]->VSSetSamplers(binding, 1, &samplerState);

        if (shaderStage & GS)
            m_Handles[m_Current]->GSSetSamplers(binding, 1, &samplerState);

        if (shaderStage & PS)
            m_Handles[m_Current]->PSSetSamplers(binding, 1, &samplerState);

        if (shaderStage & CS)
            m_Handles[m_Current]->CSSetSamplers(binding, 1, &samplerState);

        if (shaderStage & HS)
            m_Handles[m_Current]->HSSetSamplers(binding, 1, &samplerState);

        if (shaderStage & DS)
            m_Handles[m_Current]->DSSetSamplers(binding, 1, &samplerState);
    }

    void GfxContext::BindShader(GfxShader* shader)
    {
        m_Shader = shader;

        if (shader)
        {
            m_Handles[m_Current]->VSSetShader(shader->GetVS(), nullptr, 0);
            m_Handles[m_Current]->PSSetShader(shader->GetPS(), nullptr, 0);
            m_Handles[m_Current]->DSSetShader(shader->GetDS(), nullptr, 0);
            m_Handles[m_Current]->HSSetShader(shader->GetHS(), nullptr, 0);
            m_Handles[m_Current]->GSSetShader(shader->GetGS(), nullptr, 0);
            m_Handles[m_Current]->CSSetShader(shader->GetCS(), nullptr, 0);
        }
        else
        {
            m_Handles[m_Current]->VSSetShader(nullptr, nullptr, 0);
            m_Handles[m_Current]->PSSetShader(nullptr, nullptr, 0);
            m_Handles[m_Current]->DSSetShader(nullptr, nullptr, 0);
            m_Handles[m_Current]->HSSetShader(nullptr, nullptr, 0);
            m_Handles[m_Current]->GSSetShader(nullptr, nullptr, 0);
            m_Handles[m_Current]->CSSetShader(nullptr, nullptr, 0);
        }
    }

    void GfxContext::SetRenderTarget(GfxCubemapRenderTarget* cubemapRT, unsigned int face)
    {
        m_RenderTarget = nullptr;
        m_DepthStencil = nullptr;

        ID3D11RenderTargetView* rtv = cubemapRT->GetRTV(face);
        BindRT(m_Handles[m_Current], 1, &rtv, nullptr, cubemapRT->GetWidth(), cubemapRT->GetHeight());
    }

    void GfxContext::SetStencilRef(unsigned int ref)
    {
        m_StencilRef = ref;
        m_Handles[m_Current]->OMSetDepthStencilState(m_State->GetDepthStencilState(), m_StencilRef);
    }

    void GfxContext::Dispatch(unsigned int x, unsigned int y, unsigned int z)
    {
        m_Handles[m_Current]->Dispatch(x, y, z);
    }

    void GfxContext::Draw(unsigned int numVerts)
    {
        m_InputAssember.PrepareForDraw(m_Shader, m_Handles[m_Current]);
        m_Handles[m_Current]->Draw(numVerts, 0);
    }

    void GfxContext::DrawIndexed(unsigned int numIndices)
    {
        m_InputAssember.PrepareForDraw(m_Shader, m_Handles[m_Current]);
        m_Handles[m_Current]->DrawIndexed(numIndices, 0, 0);
    }

    void GfxContext::DrawInstanced(unsigned int numVerts, unsigned int numInstances)
    {
        m_InputAssember.PrepareForDraw(m_Shader, m_Handles[m_Current]);
        m_Handles[m_Current]->DrawInstanced(numVerts, numInstances, 0, 0);
    }

    void GfxContext::DrawIndexedInstanced(unsigned int numIndices, unsigned int numInstances)
    {
        m_InputAssember.PrepareForDraw(m_Shader, m_Handles[m_Current]);
        m_Handles[m_Current]->DrawIndexedInstanced(numIndices, numInstances, 0, 0, 0);
    }

    void GfxContext::DrawFullSceen()
    {
        m_InputAssember.PrepareForDraw(m_Shader, m_Handles[m_Current]);
        BindVertexBuffer(GfxDefaults::VB_2DQUAD);
        Draw(6);
    }

    void GfxContext::BeginPass(const std::string& debugName)
    {
#ifdef DEBUG
        ASSERT(!m_Deferred, "[GfxContext] Trying to add debug flag to deferred context!");
        std::wstring wDebugName = StringUtil::ToWideString(debugName);
        m_DebugMarkers->BeginEvent(wDebugName.c_str());
#endif
    }

    void GfxContext::EndPass()
    {
#ifdef DEBUG
        ASSERT(!m_Deferred, "[GfxContext] Trying to add debug flag to deferred context!");
        m_DebugMarkers->EndEvent();
#endif
    }

    void GfxContext::SubmitDeferredWork()
    {
        if (!m_Deferred) return;

        unsigned int next = (m_Current + 1) % NUM_DEFERRED_HANDLES;

        ID3D11CommandList* commandList = nullptr;
        DX_CALL(m_Handles[next]->FinishCommandList(FALSE, &commandList));
        g_Device->GetContext()->m_Handles[0]->ExecuteCommandList(commandList, TRUE);
        commandList->Release();

        SwitchCurrentHandle(next);
    }

    void GfxContext::SwitchCurrentHandle(unsigned int nextHandle)
    {
        BindState(g_Device->GetDefaultState(), nextHandle);
        SetRenderTarget(g_Device->GetFinalRT(), nextHandle);
        SetDepthStencil(g_Device->GetFinalRT(), nextHandle);

        std::vector<GfxSampler*>& defaultSamplers = g_Device->GetDefaultSamplers();
        size_t maxCustomSamplers = g_Device->GetMaxCustomSamplers();
        ID3D11SamplerState** samplers = (ID3D11SamplerState**)malloc(defaultSamplers.size() * sizeof(ID3D11SamplerState*));
        for (size_t i = 0; i < defaultSamplers.size(); i++)
        {
            samplers[i] = defaultSamplers[i]->GetSampler();
        }

        m_Handles[nextHandle]->VSSetSamplers(maxCustomSamplers, defaultSamplers.size(), samplers);
        m_Handles[nextHandle]->PSSetSamplers(maxCustomSamplers, defaultSamplers.size(), samplers);
        m_Handles[nextHandle]->GSSetSamplers(maxCustomSamplers, defaultSamplers.size(), samplers);
        m_Handles[nextHandle]->CSSetSamplers(maxCustomSamplers, defaultSamplers.size(), samplers);

        m_Current = nextHandle;
    }

    void GfxContext::BindState(GfxDeviceState* state, unsigned int handleIndex)
    {
        ASSERT(!state || state->IsCompiled(), "Trying to bind state that isn't compiled!");

        m_State = state ? state : g_Device->GetDefaultState();

        const FLOAT blendFactor[] = { 1.0f,1.0f,1.0f,1.0f };
        m_Handles[handleIndex]->RSSetState(m_State->GetRasterizerState());
        m_Handles[handleIndex]->OMSetDepthStencilState(m_State->GetDepthStencilState(), m_StencilRef);
        m_Handles[handleIndex]->OMSetBlendState(m_State->GetBlendState(), blendFactor, 0xffffffff);
    }

    void GfxContext::SetRenderTarget(GfxRenderTarget* renderTarget, unsigned int handleIndex)
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

        BindRT(m_Handles[handleIndex], numRTs, rtvs, dsv, width, height);
    }

#ifdef DEBUG
    void GfxContext::InitDebugLayer()
    {
        ID3D11Debug* d3dDebug = nullptr;
        g_Device->GetDevice()->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
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

        DX_CALL(m_Handles[m_Current]->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&m_DebugMarkers));
    }
#endif // DEBUG

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

        CreateSwapChain();
        InitSamplers();
        m_DefaultState.Compile();
        m_Contexts[CURRENT_THREAD] = new GfxContext{ m_ImmediateContext };
        GfxDefaults::InitDefaults();
        g_GUI = new GUI(Window::Get()->GetHandle(), m_Device, m_ImmediateContext);
        m_Initialized = true;
    }

    GfxDevice::~GfxDevice()
    {
        GfxDefaults::DestroyDefaults();

        delete m_FinalRT;
        
        for (auto& it : m_Contexts) delete it.second;
        for (GfxSampler* sampler : m_Samplers) delete sampler;
        m_SwapChain->Release();
        m_Device->Release();
        m_DefaultState.~GfxDeviceState();

#ifdef DEBUG
        ID3D11Debug* d3dDebug = nullptr;
        m_Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
        d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_IGNORE_INTERNAL | D3D11_RLDO_DETAIL);
#endif
    }

    void GfxDevice::EndFrame()
    {
        // Submit contexts
        for (auto& it : m_Contexts) it.second->SubmitDeferredWork();

        // Delete contexts that requested it
        auto& contexts = m_Contexts;
        const auto deleteContexts = [&contexts](ThreadID threadID) {
            delete contexts[threadID];
            contexts.erase(threadID);
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

        DX_CALL(baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&m_ImmediateContext));
        baseDeviceContext->Release();

        return true;
    }

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

    void GfxDevice::InitSamplers()
    {
        m_Samplers.resize(4);
        m_Samplers[0] = new GfxSampler(SamplerFilter::Point, SamplerMode::Border);
        m_Samplers[1] = new GfxSampler(SamplerFilter::Linear, SamplerMode::Border);
        m_Samplers[2] = new GfxSampler(SamplerFilter::Linear, SamplerMode::Clamp);
        m_Samplers[3] = new GfxSampler(SamplerFilter::Linear, SamplerMode::Wrap);

        m_MaxCustomSamplers = 16 - m_Samplers.size();
    }
}


