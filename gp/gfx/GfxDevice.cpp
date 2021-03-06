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
            GfxTexture2D* CreateColorTexture(GfxContext* context, ColorUNORM color)
            {
                GfxTexture2D* texture = new GfxTexture2D(1, 1);
                context->UploadToTexture(texture, &color);
                return texture;
            }
        }

        void InitDefaults(GfxContext* context)
        {
            VB_CUBE = new GfxVertexBuffer<Data::VB_CUBE_TYPE>((void*) Data::VB_CUBE_DATA, Data::VB_CUBE_SIZE);
            VB_2DQUAD = new GfxVertexBuffer<Data::VB_QUAD2D_TYPE>((void*) Data::VB_QUAD2D_DATA, Data::VB_QUAD2D_SIZE);
            VB_QUAD = new GfxVertexBuffer<Data::VB_QUAD_TYPE>((void*) Data::VB_QUAD_DATA, Data::VB_QUAD_SIZE);

            TEX2D_WHITE = CreateColorTexture(context, { 255, 255, 255, 255 });
            TEX2D_BLACK = CreateColorTexture(context, { 0, 0, 0, 255 });
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

    GfxContext::GfxContext():
        m_Deferred(true)
    {
        ID3D11Device1* d = g_Device->GetDevice();
        DX_CALL(d->CreateDeferredContext1(0, &m_Handle));
        Reset();
    }

    GfxContext::GfxContext(ID3D11DeviceContext1* context):
        m_Deferred(false)
    {
        ASSERT(context, "[GfxContext] Trying to initialize immediate context with null");
        m_Handle = context;
        Reset();

#ifdef DEBUG
        InitDebugLayer();
#endif
    }

    GfxContext::~GfxContext()
    {
        if (m_Deferred)
        {
            Submit();
            m_Handle->Release();
        }
    }

    void GfxContext::GenerateMips(GfxBaseTexture2D* texture)
    {
        ContextOperation(this, "Generate mips");
        m_Handle->GenerateMips(GetDeviceSRV(this, texture));
    }

    DXGI_FORMAT ToDXGIFormat(TextureFormat format);


    void GfxContext::ResolveMSResource(GfxRenderTarget* srcResource, GfxBaseTexture2D* dstResource)
    {
        ASSERT(srcResource->GetNumRTs() == dstResource->GetResource()->GetArraySize(), "[GfxContext::ResolveMSResouce] Source num render targets must match Destination array size!");
        ASSERT(srcResource->GetResource()->GetNumSamples() > 1 && dstResource->GetResource()->GetNumSamples() == 1, "[GfxContext::ResolveMSReource] Number of samples for src must be > 1 and for dst must be == 1");
        ASSERT(srcResource->GetResource()->GetFormat() == dstResource->GetResource()->GetFormat(), "[GfxContext::ResolveMSReource] Texutre formats of src and dst must match!");

        ContextOperation(this, "Resolve multisample resource");

        for (unsigned int i = 0; i < srcResource->GetNumRTs(); i++)
        {
            unsigned int dstSubresourceIndex = D3D11CalcSubresource(0, i, dstResource->GetResource()->GetNumMips());
            m_Handle->ResolveSubresource(dstResource->GetResource()->GetHandle(), dstSubresourceIndex, srcResource->GetResource(i)->GetHandle(), 0, ToDXGIFormat(dstResource->GetResource()->GetFormat()));
        }
    }

    void GfxContext::UploadToTexture(TextureResource2D* textureResource, void* data, unsigned int arrayIndex)
    {
        ContextOperation(this, "Upload to texture");
        unsigned int subresourceIndex = D3D11CalcSubresource(0, arrayIndex, textureResource->GetNumMips());
        m_Handle->UpdateSubresource(textureResource->GetHandle(), subresourceIndex, nullptr, data, textureResource->GetRowPitch(), 0u);
    }

    static D3D11_MAP GetMapFlags(bool read, bool write)
    {
        if (read && write) return D3D11_MAP_READ_WRITE;
        else if (read) return D3D11_MAP_READ;
        else if (write) return D3D11_MAP_WRITE_DISCARD;

        NOT_IMPLEMENTED;
        return D3D11_MAP_READ;
    }

    void* GfxContext::Map(GfxBuffer* gfxBuffer, bool read, bool write)
    {
        ContextOperation(this, "Map");
        if (!gfxBuffer->Initialized())
        {
            if(write) gfxBuffer->AddCreationFlags(RCF_CPUWrite);
            if (read) gfxBuffer->AddCreationFlags(RCF_CPURead);
            gfxBuffer->Initialize(this);
        }

        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        DX_CALL(m_Handle->Map(GetDeviceHandle(this, gfxBuffer), 0, GetMapFlags(read, write), 0, &mappedSubresource));
        return mappedSubresource.pData;
    }

    void GfxContext::Unmap(GfxBuffer* gfxBuffer)
    {
        ContextOperation(this, "Unmap");
        m_Handle->Unmap(GetDeviceHandle(this, gfxBuffer), 0);
    }

    void GfxContext::Clear(const Vec4& color)
    {
        ContextOperation(this, "Clear");
        const FLOAT clearColor[4] = { color.x, color.y, color.z, color.w };

        if (m_RenderTarget)
        {
            for (size_t i = 0; i < m_RenderTarget->GetNumRTs(); i++)
            {
                m_Handle->ClearRenderTargetView(m_RenderTarget->GetRTV(i), clearColor);
            }
        }

        if (m_DepthStencil)
        {
            m_Handle->ClearDepthStencilView(m_DepthStencil->GetDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        }
    }

    void GfxContext::BindShader(GfxShader* shader)
    {
        ContextOperation(this, "Bind shader");
        m_Shader = shader;
        m_ReloadShader = true;
    }

    void GfxContext::SetRenderTarget(GfxCubemapRenderTarget* cubemapRT, unsigned int face)
    {
        ContextOperation(this, "Bind render target");

        // HACK
        m_RenderTarget = nullptr;
        m_DepthStencil = nullptr;

        if (!cubemapRT->Initialized()) cubemapRT->Initialize(this);

        ID3D11RenderTargetView* rtv = cubemapRT->GetRTV(face);
        BindRT(m_Handle, 1, &rtv, nullptr, cubemapRT->GetWidth(), cubemapRT->GetHeight());

        m_ReloadShader = true;
    }

    void GfxContext::SetStencilRef(unsigned int ref)
    {
        ContextOperation(this, "Set stencil ref");
        m_StencilRef = ref;
        m_ReloadShader = true;
    }

    void GfxContext::Dispatch(unsigned int x, unsigned int y, unsigned int z)
    {
        ContextOperation(this, "Dispatch");
        if (m_ReloadShader) BindShaderToPipeline();
        m_Handle->Dispatch(x, y, z);
    }

    void GfxContext::Draw(unsigned int numVerts)
    {
        ContextOperation(this, "Draw");
        if (m_ReloadShader) BindShaderToPipeline();
        m_InputAssember.PrepareForDraw(m_Shader, m_Handle);
        m_Handle->Draw(numVerts, 0);
    }

    void GfxContext::DrawIndexed(unsigned int numIndices)
    {
        ContextOperation(this, "DrawIndexed");
        if (m_ReloadShader) BindShaderToPipeline();
        m_InputAssember.PrepareForDraw(m_Shader, m_Handle);
        m_Handle->DrawIndexed(numIndices, 0, 0);
    }

    void GfxContext::DrawInstanced(unsigned int numVerts, unsigned int numInstances)
    {
        ContextOperation(this, "DrawInstanced");
        m_InputAssember.PrepareForDraw(m_Shader, m_Handle);
        if (m_ReloadShader) BindShaderToPipeline();
        m_Handle->DrawInstanced(numVerts, numInstances, 0, 0);
    }

    void GfxContext::DrawIndexedInstanced(unsigned int numIndices, unsigned int numInstances)
    {
        ContextOperation(this, "DrawIndexedInstanced");
        if (m_ReloadShader) BindShaderToPipeline();
        m_InputAssember.PrepareForDraw(m_Shader, m_Handle);
        m_Handle->DrawIndexedInstanced(numIndices, numInstances, 0, 0, 0);
    }

    void GfxContext::DrawFC()
    {
        ContextOperation(this, "Draw FC");
        BindVertexBuffer(GfxDefaults::VB_2DQUAD);
        Draw(6);
    }

    void GfxContext::BeginPass(const std::string& debugName)
    {
#ifdef DEBUG
        ContextOperation(this, "BeginPass");
        ASSERT(!m_Deferred, "[GfxContext] Trying to add debug flag to deferred context!");
        std::wstring wDebugName = StringUtil::ToWideString(debugName);
        m_DebugMarkers->BeginEvent(wDebugName.c_str());
#endif
    }

    void GfxContext::EndPass()
    {
#ifdef DEBUG
        ContextOperation(this, "EndPass");
        ASSERT(!m_Deferred, "[GfxContext] Trying to add debug flag to deferred context!");
        m_DebugMarkers->EndEvent();
#endif
    }

    void GfxContext::Submit()
    {
        ContextOperation(this, "Submit");
        if(g_Device) g_Device->SubmitContext(*this);

        m_Handle->Release();
        DX_CALL(g_Device->GetDevice()->CreateDeferredContext1(0, &m_Handle));
        Reset();
    }

    ID3D11CommandList* GfxContext::CreateCommandList() const
    {
        //ContextOperation(this, "CreateCommandList");
        ASSERT(m_Deferred, "[GfxContext] Cannot submit immediate context!");
        
        ID3D11CommandList* commandList = nullptr;
        DX_CALL(m_Handle->FinishCommandList(FALSE, &commandList));
        return commandList;
    }

    void GfxContext::Reset()
    {
        if (!g_Device) return;

        ContextOperation(this, "Reset context");

        SetRenderTarget(g_Device->GetFinalRT());
        SetDepthStencil(g_Device->GetFinalRT());

        std::vector<GfxSampler*>& defaultSamplers = g_Device->GetDefaultSamplers();
        size_t maxCustomSamplers = g_Device->GetMaxCustomSamplers();
        ID3D11SamplerState** samplers = (ID3D11SamplerState**)malloc(defaultSamplers.size() * sizeof(ID3D11SamplerState*));
        for (size_t i = 0; i < defaultSamplers.size(); i++)
        {
            samplers[i] = defaultSamplers[i]->GetSampler();
        }

        m_Handle->VSSetSamplers(maxCustomSamplers, defaultSamplers.size(), samplers);
        m_Handle->PSSetSamplers(maxCustomSamplers, defaultSamplers.size(), samplers);
        m_Handle->GSSetSamplers(maxCustomSamplers, defaultSamplers.size(), samplers);
        m_Handle->CSSetSamplers(maxCustomSamplers, defaultSamplers.size(), samplers);
    }

    void GfxContext::BindUAV(ID3D11DeviceContext1* context, unsigned int shaderStage, ID3D11UnorderedAccessView* uav, unsigned int binding)
    {
        ASSERT(shaderStage == CS, "[NOT_SUPPORTED] Trying to bind RW resource to stage that isn't compute shader.");
        context->CSSetUnorderedAccessViews(binding, 1, &uav, nullptr);
    }

    void GfxContext::BindSRV(ID3D11DeviceContext1* context, unsigned int shaderStage, ID3D11ShaderResourceView* srv, unsigned int binding)
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

    void GfxContext::BindCB(ID3D11DeviceContext1* context, unsigned int shaderStage, ID3D11Buffer* buffer, unsigned int binding)
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

    void GfxContext::BindRT(ID3D11DeviceContext1* context, unsigned int numRTs, ID3D11RenderTargetView** rtvs, ID3D11DepthStencilView* dsv, int width, int height)
    {
        const D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
        if (width > 0) context->RSSetViewports(1, &viewport);
        context->OMSetRenderTargets(numRTs, rtvs, dsv);        m_ReloadShader = true;
    }

    void GfxContext::BindSamplerState(ID3D11DeviceContext1* context, unsigned int shaderStage, ID3D11SamplerState* sampler, unsigned int binding)
    {
        ASSERT(binding < g_Device->GetMaxCustomSamplers(), "[GfxDevice::BindSampler] " + std::to_string(binding) + " is out of the limit, maximum binding is " + std::to_string(g_Device->GetMaxCustomSamplers() - 1));

        if (shaderStage & VS)
            context->VSSetSamplers(binding, 1, &sampler);

        if (shaderStage & GS)
            context->GSSetSamplers(binding, 1, &sampler);

        if (shaderStage & PS)
            context->PSSetSamplers(binding, 1, &sampler);

        if (shaderStage & CS)
            context->CSSetSamplers(binding, 1, &sampler);

        if (shaderStage & HS)
            context->HSSetSamplers(binding, 1, &sampler);

        if (shaderStage & DS)
            context->DSSetSamplers(binding, 1, &sampler);
    }

    void GfxContext::BindDeviceState(GfxDeviceState* deviceState)
    {
        ContextOperation(this, "Bind device state");
        const FLOAT blendFactor[] = { 1.0f,1.0f,1.0f,1.0f };
        m_Handle->RSSetState(deviceState->Rasterizer);
        m_Handle->OMSetDepthStencilState(deviceState->DepthStencil, m_StencilRef);
        m_Handle->OMSetBlendState(deviceState->Blend, blendFactor, 0xffffffff);
        m_Handle->IASetPrimitiveTopology(ToDXTopology(deviceState->Topology));
    }

    void GfxContext::BindShaderToPipeline()
    {
        ContextOperation(this, "Bind shader to pipeline");
        if (m_Shader && !m_Shader->IsInitialized())
        {
            m_Shader->Initialize();
            ASSERT(m_Shader->IsInitialized(), "[GfxContext] ASSERT Failed: shader->IsInitialized()");
        }

        if (m_Shader)
        {
            m_Handle->VSSetShader(m_Shader->GetVS(), nullptr, 0);
            m_Handle->PSSetShader(m_Shader->GetPS(), nullptr, 0);
            m_Handle->DSSetShader(m_Shader->GetDS(), nullptr, 0);
            m_Handle->HSSetShader(m_Shader->GetHS(), nullptr, 0);
            m_Handle->GSSetShader(m_Shader->GetGS(), nullptr, 0);
            m_Handle->CSSetShader(m_Shader->GetCS(), nullptr, 0);

            if (m_RenderTarget && m_RenderTarget->GetResource()->GetNumSamples() > 1)
                BindDeviceState(m_Shader->GetDeviceStateMS());
            else
                BindDeviceState(m_Shader->GetDeviceState());
        }
        else
        {
            m_Handle->VSSetShader(nullptr, nullptr, 0);
            m_Handle->PSSetShader(nullptr, nullptr, 0);
            m_Handle->DSSetShader(nullptr, nullptr, 0);
            m_Handle->HSSetShader(nullptr, nullptr, 0);
            m_Handle->GSSetShader(nullptr, nullptr, 0);
            m_Handle->CSSetShader(nullptr, nullptr, 0);

            // Also unbind states ?
        }

        m_ReloadShader = false;
    }

    void GfxContext::SetRenderTarget(GfxRenderTarget* renderTarget)
    {
        if (!renderTarget->Initialized()) renderTarget->Initialize(this);

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

        BindRT(m_Handle, numRTs, rtvs, dsv, width, height);

        m_ReloadShader = true;
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

        DX_CALL(m_Handle->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&m_DebugMarkers));
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
        m_ImmediateContext = new GfxContext{ m_DeviceContext };
        GfxDefaults::InitDefaults(m_ImmediateContext);
        g_GUI = new GUI(Window::Get()->GetHandle(), m_Device, m_ImmediateContext->GetHandle());
        m_Initialized = true;
    }

    GfxDevice::~GfxDevice()
    {
        GfxDefaults::DestroyDefaults();

        delete m_FinalRT;
        for (GfxSampler* sampler : m_Samplers) delete sampler;
        m_SwapChain->Release();
        delete m_ImmediateContext;
        m_Device->Release();

#ifdef DEBUG
        ID3D11Debug* d3dDebug = nullptr;
        m_Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
        d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_IGNORE_INTERNAL | D3D11_RLDO_DETAIL);
#endif
    }

    void GfxDevice::RecreateSwapchain()
    {
        m_SwapChain->Release();
        CreateSwapChain();
    }

    void GfxDevice::EndFrame()
    {
        // Execute pending command lists
        m_PendingCommandLists.ForEachAndClear([this](ID3D11CommandList* cmdList) {
            m_ImmediateContext->GetHandle()->ExecuteCommandList(cmdList, TRUE);
            });
        m_PendingCommandLists.Clear();

        // Present to screen
        m_SwapChain->Present(GlobalVariables::GP_CONFIG.VSYNC ? 1 : 0, 0);
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

            DX_CALL(dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory));
            dxgiAdapter->Release();
        }

        DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
        d3d11SwapChainDesc.Width = GlobalVariables::GP_CONFIG.WindowWidth;
        d3d11SwapChainDesc.Height = GlobalVariables::GP_CONFIG.WindowHeight;
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


