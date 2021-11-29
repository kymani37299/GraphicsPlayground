#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "core/Threads.h"
#include "gfx/GfxCommon.h"
#include "gfx/GfxBuffers.h"
#include "gfx/GfxTexture.h"
#include "gfx/GfxDefaultsData.h"

struct ID3D11Device1;
struct ID3D11DeviceContext1;
struct IDXGISwapChain1;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState1;
struct ID3D11BlendState1;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11SamplerState;
struct ID3D11CommandList;
#ifdef DEBUG
struct ID3DUserDefinedAnnotation;
#endif

namespace GP
{
	class GfxShader;

	///////////////////////////////////////
	//			MODEL					//
	/////////////////////////////////////

	enum ShaderStage
	{
		VS = 1,
		GS = 2,
		PS = 4,
		CS = 8,
		HS = 16,
		DS = 32
	};

	///////////////////////////////////////
	//			Defaults				//
	/////////////////////////////////////

	namespace GfxDefaults
	{
		GP_DLL extern GfxVertexBuffer<Data::VB_CUBE_TYPE>* VB_CUBE;
		GP_DLL extern GfxVertexBuffer<Data::VB_QUAD2D_TYPE>* VB_2DQUAD;
		GP_DLL extern GfxVertexBuffer<Data::VB_QUAD_TYPE>* VB_QUAD;

		GP_DLL extern GfxTexture2D* TEX2D_WHITE;
		GP_DLL extern GfxTexture2D* TEX2D_BLACK;

		void InitDefaults();
		void DestroyDefaults();
	}

	///////////////////////////////////////
	//			Helpers					//
	/////////////////////////////////////

	namespace
	{
		template<typename ResourceType>
		inline ID3D11Buffer* GetDeviceHandle(ResourceType* resource)
		{
			if (!resource) return nullptr;

			if (!resource->Initialized())
				resource->Initialize();

			ASSERT(resource->GetResource()->GetHandle(), "[GetDeviceHandle] resource->GetBuffer() == nullptr");
			return resource->GetResource()->GetHandle();
		}

		template<typename ResourceType>
		inline ID3D11ShaderResourceView* GetDeviceSRV(ResourceType* resource)
		{
			if (!resource) return nullptr;

			if (!resource->Initialized())
				resource->Initialize();

			ASSERT(resource->GetSRV(), "[GetDeviceSRV] resource->GetSRV() == nullptr");
			return resource->GetSRV();
		}

		template<typename ResourceType>
		inline ID3D11UnorderedAccessView* GetDeviceUAV(ResourceType* resource)
		{
			if (!resource) return nullptr;

			if (!resource->Initialized())
				resource->Initialize();

			ASSERT(resource->GetUAV(), "[GetDeviceUAV] resource->GetUAV() == nullptr");
			return resource->GetUAV();
		}
	}

	///////////////////////////////////////
	//			CORE					//
	/////////////////////////////////////

	class GfxInputAssembler
	{
	public:
		inline void BindVertexBuffer(unsigned int slot, GfxBuffer* gfxBuffer, unsigned int stride, unsigned int offset)
		{
			if (gfxBuffer)
			{
				if (m_VBResources.size() <= slot)
				{
					m_VBResources.resize(slot + 1, nullptr);
					m_VBStrides.resize(slot + 1, 0);
					m_VBOffsets.resize(slot + 1, 0);
				}

				m_VBResources[slot] = GetDeviceHandle(gfxBuffer);
				m_VBStrides[slot] = stride;
				m_VBOffsets[slot] = offset;
			}
			else
			{
				m_VBResources.clear();
				m_VBStrides.clear();
				m_VBOffsets.clear();
			}
			m_Dirty = true;
		}

		inline void BindIndexBuffer(GfxIndexBuffer* indexBuffer)
		{
			if (indexBuffer)
			{
				m_IBResource = GetDeviceHandle(indexBuffer);
				m_IBStride = indexBuffer->GetStride();
				m_IBOffset = indexBuffer->GetOffset();
			}
			else
			{
				m_IBResource = nullptr;
				m_IBStride = 0;
				m_IBOffset = 0;
			}
			m_Dirty = true;
		}

		void PrepareForDraw(GfxShader* shader, ID3D11DeviceContext1* context);

	private:
		bool m_Dirty = true;

		std::vector<ID3D11Buffer*> m_VBResources;
		std::vector<unsigned int> m_VBStrides;
		std::vector<unsigned int> m_VBOffsets;

		ID3D11Buffer* m_IBResource = nullptr;
		unsigned int m_IBStride = 0;
		unsigned int m_IBOffset = 0;
	};

	class GfxContext
	{
		friend class GfxDevice;
	public:
		
		// NOTE: VertexBuffer - Binding one slot to null will clear whole vertex assembly
		template<typename T> inline void BindVertexBuffer(GfxVertexBuffer<T>* vertexBuffer);
		template<typename T> inline void BindVertexBufferSlot(GfxVertexBuffer<T>* vertexBuffer, unsigned int slot);
		inline void BindVertexBufferSlot(std::nullptr_t, unsigned int slot);
		template<typename T> inline void BindInstanceBuffer(GfxInstanceBuffer<T>* instanceBuffer);
		template<typename T> inline void BindInstanceBufferSlot(GfxInstanceBuffer<T>* instanceBuffer, unsigned int slot);
		inline void BindInstanceBufferSlot(std::nullptr_t, unsigned int slot);
		inline void BindIndexBuffer(GfxIndexBuffer* indexBuffer);
		inline void BindConstantBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding);
		inline void BindStructuredBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding);
		inline void BindRWStructuredBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding);
		inline void BindTexture2D(unsigned int shaderStage, GfxBaseTexture2D* texture, unsigned int binding);
		inline void BindTexture3D(unsigned int shaderStage, GfxBaseTexture3D* texture, unsigned int binding);
		inline void BindRWTexture3D(unsigned int shaderStage, GfxBaseTexture3D* texture, unsigned int binding);
		inline void BindTextureArray2D(unsigned int shaderStage, GfxBaseTexture2D* texture, unsigned int binding);
		inline void BindCubemap(unsigned int shaderStage, GfxBaseTexture2D* texture, unsigned int binding);
		inline void UnbindTexture(unsigned int shaderStage, unsigned int binding);
		inline void BindSampler(unsigned int shaderStage, GfxSampler* sampler, unsigned int binding);
		inline void SetDepthStencil(GfxRenderTarget* depthStencil);

		GP_DLL void Clear(const Vec4& color = VEC4_ZERO);
		GP_DLL void BindShader(GfxShader* shader);

		GP_DLL void SetRenderTarget(GfxCubemapRenderTarget* cubemapRT, unsigned int face);
		GP_DLL void SetRenderTarget(GfxRenderTarget* renderTarget);
		GP_DLL void SetStencilRef(unsigned int ref);

		GP_DLL void Dispatch(unsigned int x = 1, unsigned int y = 1, unsigned int z = 1);
		GP_DLL void Draw(unsigned int numVerts);
		GP_DLL void DrawIndexed(unsigned int numIndices);
		GP_DLL void DrawInstanced(unsigned int numVerts, unsigned int numInstances);
		GP_DLL void DrawIndexedInstanced(unsigned int numIndices, unsigned int numInstances);
		GP_DLL void DrawFC();

		GP_DLL void BeginPass(const std::string& debugName);
		GP_DLL void EndPass();

		GP_DLL void Submit();

		inline GfxRenderTarget* GetRenderTarget() const { return m_RenderTarget; }
		inline GfxRenderTarget* GetDepthStencil() const { return m_DepthStencil; }
		
		// HACK: This should never be used
		inline ID3D11DeviceContext1* GetHandle() const { return m_Handle; }

	private:
		GP_DLL GfxContext();
		GfxContext(ID3D11DeviceContext1* context);

		GP_DLL ~GfxContext();

		GP_DLL ID3D11CommandList* CreateCommandList() const;
		GP_DLL void Reset();
		
		GP_DLL void BindUAV(ID3D11DeviceContext1* context, unsigned int shaderStage, ID3D11UnorderedAccessView* uav, unsigned int binding);
		GP_DLL void BindSRV(ID3D11DeviceContext1* context, unsigned int shaderStage, ID3D11ShaderResourceView* srv, unsigned int binding);
		GP_DLL void BindCB(ID3D11DeviceContext1* context, unsigned int shaderStage, ID3D11Buffer* buffer, unsigned int binding);
		GP_DLL void BindRT(ID3D11DeviceContext1* context, unsigned int numRTs, ID3D11RenderTargetView** rtvs, ID3D11DepthStencilView* dsv, int width, int height);
		GP_DLL void BindSamplerState(ID3D11DeviceContext1* context, unsigned int shaderStage, ID3D11SamplerState* sampler, unsigned int binding);

#ifdef DEBUG
		void InitDebugLayer();
#endif

	private:
		bool m_Deferred = true;
		ID3D11DeviceContext1* m_Handle;

		// Current State
		unsigned int m_StencilRef = 0xff;
		GfxInputAssembler m_InputAssember;
		GfxRenderTarget* m_RenderTarget = nullptr;
		GfxRenderTarget* m_DepthStencil = nullptr;
		GfxShader* m_Shader = nullptr;

#ifdef DEBUG
		ID3DUserDefinedAnnotation* m_DebugMarkers;
#endif
	};

	class GfxDevice
	{
		DELETE_COPY_CONSTRUCTOR(GfxDevice);
	public:
		GfxDevice();
		void Init();
		~GfxDevice();

		void EndFrame();

		inline bool IsInitialized() const { return m_Initialized; }

		inline ID3D11Device1* GetDevice() const { return m_Device; }

		inline GfxContext* GetContext() 
		{
			ASSERT(m_Contexts[CURRENT_THREAD], "[GfxContext] There is no context for this thread. Did you forgot to call g_Device->CreateDeferredContext ?");
			return m_Contexts[CURRENT_THREAD]; 
		}

		inline GfxContext* CreateDeferredContext()
		{
			ASSERT(!m_Contexts[CURRENT_THREAD], "[GfxContext] Trying to create more than one deferred context per thread.");
			m_Contexts[CURRENT_THREAD] = new GfxContext();
			return m_Contexts[CURRENT_THREAD];
		}

		inline void DeleteDeferredContext()
		{
			ASSERT(m_Contexts[CURRENT_THREAD], "[GfxContext] Trying to delete deferred context on thread that never created it.");
			delete m_Contexts[CURRENT_THREAD];
			m_Contexts[CURRENT_THREAD] = nullptr;
		}

		inline void SubmitContext(GfxContext& context)
		{
			m_PendingCommandLists.Add(context.CreateCommandList());
			context.Reset();
		}

		inline size_t GetMaxCustomSamplers() const { return m_MaxCustomSamplers; }
		inline std::vector<GfxSampler*>& GetDefaultSamplers() { return m_Samplers; }
		inline GfxRenderTarget* GetFinalRT() const { return m_FinalRT; }

	private:
		bool CreateDevice();
		void CreateSwapChain();

		void InitSamplers();

	private:
		bool m_Initialized = false;

		// Handles
		ID3D11Device1* m_Device;
		ID3D11DeviceContext1* m_ImmediateContext;
		IDXGISwapChain1* m_SwapChain;
		
		// Context
		std::unordered_map<ThreadID, GfxContext*> m_Contexts;
		MutexVector<ID3D11CommandList*> m_PendingCommandLists;

		// Default state
		GfxRenderTarget* m_FinalRT;

		// Statics
		unsigned int m_MaxCustomSamplers;
		std::vector<GfxSampler*> m_Samplers;
	};

	GP_DLL extern GfxDevice* g_Device;
}

#include "GfxDevice.hpp"