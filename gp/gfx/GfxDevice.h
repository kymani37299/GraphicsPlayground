#pragma once

#include <vector>
#include <string>
#include <thread>

#include "gfx/GfxCommon.h"
#include "gfx/GfxBuffers.h"
#include "gfx/GfxDefaultsData.h"

struct ID3D11Device1;
struct ID3D11DeviceContext1;
struct IDXGISwapChain1;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState1;
struct ID3D11BlendState1;
struct ID3D11ShaderResourceView;
#ifdef DEBUG
struct ID3DUserDefinedAnnotation;
#endif

namespace GP
{
	class GfxShader;
	class GfxComputeShader;
	class GfxBufferResource;
	class GfxTexture2D;
	class GfxTextureArray2D;
	class GfxCubemap;
	class GfxRenderTarget;
	class GfxCubemapRenderTarget;
	class GfxSampler;
	class ShaderFactory;

	///////////////////////////////////////
	//			MODEL					//
	/////////////////////////////////////

	enum ShaderStage
	{
		VS = 1,
		GS = 2,
		PS = 4,
		CS = 8,
		// 16
	};

	enum class  StencilOp
	{
		Discard,
		Keep,
		Replace
	};

	enum class CompareOp
	{
		Always,
		Equals,
		Less
	};

	enum class Blend
	{
		Zero,
		One,
		SrcColor,
		SrcColorInv,
		SrcAlpha,
		SrcAlphaInv,
		SrcAlphaSat,
		Src1Color,
		Src1ColorInv,
		Src1Alpha,
		Src1AlphaInv,
		DestColor,
		DestColorInv,
		DestAlpha,
		DestAlphaInv,
		BlendFactor,
		BlendFactorInv
	};

	enum class BlendOp
	{
		Add,
		Substract,
		SubstractInv,
		Min,
		Max
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
	}

	///////////////////////////////////////
	//			CORE					//
	/////////////////////////////////////

	class GfxDeviceState
	{
		DELETE_COPY_CONSTRUCTOR(GfxDeviceState);
	public:
		GP_DLL GfxDeviceState() {}
		GP_DLL ~GfxDeviceState();

		inline void EnableBackfaceCulling(bool value) { m_BackfaceCullingEnabled = value; }
		inline void EnableWireframeMode(bool value) { m_WireframeModeEnabled = value; }
		inline void EnableMultisample(bool value) { m_MultisamplingEnabled = value; }
		inline void EnableDepthTest(bool value) { m_DepthEnabled = value; }
		inline void EnableDepthWrite(bool value) { m_DepthWriteEnabled = value; }
		inline void SetDepthCompareOp(CompareOp depthCompareOp) { m_DepthCompareOp = depthCompareOp; }
		inline void EnableStencil(bool value) { m_StencilEnabled = value; }
		inline void SetStencilReadMask(unsigned int readMask) { m_StencilRead = readMask; }
		inline void SetStencilWriteMask(unsigned int writeMask) { m_StencilWrite = writeMask; }
		inline void SetStencilOp(StencilOp fail, StencilOp depthFail, StencilOp pass) { m_StencilOp[0] = fail; m_StencilOp[1] = depthFail; m_StencilOp[2] = pass; }
		inline void SetStencilCompareOp(CompareOp stencilCompareOp) { m_StencilCompareOp = stencilCompareOp; }
		inline void EnableAlphaBlend(bool value) { m_AlphaBlendEnabled = value; }
		inline void SetBlendOp(BlendOp blendOp) { m_BlendOp = blendOp; }
		inline void SetBlendAlphaOp(BlendOp blendAlphaOp) { m_BlendAlphaOp = blendAlphaOp; }
		inline void SetSourceColorBlend(Blend sourceColorBlend) { m_SourceColorBlend = sourceColorBlend; }
		inline void SetDestColorBlend(Blend destColorBlend) { m_DestColorBlend = destColorBlend; }
		inline void SetSourceAlphaBlend(Blend sourceAlphaBlend) { m_SourceAlphaBlend = sourceAlphaBlend; }
		inline void SetDestAlphaBlend(Blend destAlphaBlend) { m_DestAlphaBlend = destAlphaBlend; }

		GP_DLL void Compile();

		inline ID3D11DepthStencilState* GetDepthStencilState() const { return m_DepthStencilState; }
		inline ID3D11RasterizerState1* GetRasterizerState() const { return m_RasterizerState; }
		inline ID3D11BlendState1* GetBlendState() const { return m_BlendState; }

		inline bool IsCompiled() const { return m_Compiled; }

	private:
		// Rasterizer state
		bool m_BackfaceCullingEnabled = true;
		bool m_WireframeModeEnabled = false;
		bool m_MultisamplingEnabled = false;

		// Depth stencil state
		bool m_DepthEnabled = false;
		bool m_DepthWriteEnabled = true;
		CompareOp m_DepthCompareOp = CompareOp::Less;
		bool m_StencilEnabled = false;
		unsigned int m_StencilRead = 0xff;
		unsigned int m_StencilWrite = 0xff;
		StencilOp m_StencilOp[3] = { StencilOp::Keep, StencilOp::Keep, StencilOp::Keep };
		CompareOp m_StencilCompareOp = CompareOp::Always;

		// Blend state
		bool m_AlphaBlendEnabled = false;
		BlendOp m_BlendOp = BlendOp::Add;
		BlendOp m_BlendAlphaOp = BlendOp::Add;
		Blend m_SourceColorBlend = Blend::SrcAlpha;
		Blend m_DestColorBlend = Blend::SrcAlphaInv;
		Blend m_SourceAlphaBlend = Blend::One;
		Blend m_DestAlphaBlend = Blend::One;

	private:
		bool m_Compiled = false;
		ID3D11DepthStencilState* m_DepthStencilState = nullptr;
		ID3D11RasterizerState1* m_RasterizerState = nullptr;
		ID3D11BlendState1* m_BlendState = nullptr;
	};

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

				m_VBResources[slot] = GetDeviceBuffer(gfxBuffer->GetBufferResource());
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
				m_IBResource = GetDeviceBuffer(indexBuffer->GetBufferResource());
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

		void PrepareForDraw(GfxShader* shader);

	private:
		bool m_Dirty = true;

		std::vector<ID3D11Buffer*> m_VBResources;
		std::vector<unsigned int> m_VBStrides;
		std::vector<unsigned int> m_VBOffsets;

		ID3D11Buffer* m_IBResource = nullptr;
		unsigned int m_IBStride = 0;
		unsigned int m_IBOffset = 0;
	};

	class GfxDevice
	{
		DELETE_COPY_CONSTRUCTOR(GfxDevice);
	public:
		GfxDevice();
		void Init();
		~GfxDevice();

		template<typename T>
		inline void BindVertexBuffer(GfxVertexBuffer<T>* vertexBuffer)
		{
			m_InputAssember.BindVertexBuffer(0, nullptr, 0, 0);
			m_InputAssember.BindVertexBuffer(0, vertexBuffer, vertexBuffer->GetStride(), vertexBuffer->GetOffset());
		}

		template<typename T>
		inline void BindVertexBufferSlot(GfxVertexBuffer<T>* vertexBuffer, unsigned int slot)
		{
			m_InputAssember.BindVertexBuffer(slot, vertexBuffer, vertexBuffer->GetStride(), vertexBuffer->GetOffset());
		}

		// NOTE: Binding one slot to null will clear whole vertex assembly
		inline void BindVertexBufferSlot(std::nullptr_t, unsigned int slot)
		{
			m_InputAssember.BindVertexBuffer(slot, nullptr, 0, 0);
		}

		inline void BindIndexBuffer(GfxIndexBuffer* indexBuffer)
		{
			m_InputAssember.BindIndexBuffer(indexBuffer);
		}

		GP_DLL void Clear(const Vec4& color = VEC4_ZERO);
		GP_DLL void BindState(GfxDeviceState* state);
		GP_DLL void BindConstantBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding);
		GP_DLL void BindStructuredBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding);
		GP_DLL void BindRWStructuredBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding);
		GP_DLL void BindTexture2D(unsigned int shaderStage, GfxTexture2D* texture, unsigned int binding);
		GP_DLL void BindTextureArray2D(unsigned int shaderStage, GfxTextureArray2D* textureArray, unsigned int binding);
		GP_DLL void BindCubemap(unsigned int shaderStage, GfxCubemap* cubemap, unsigned int binding);
		GP_DLL void UnbindTexture(unsigned int shaderStage, unsigned int binding);
		GP_DLL void BindSampler(unsigned int shaderStage, GfxSampler* sampler, unsigned int binding);
		GP_DLL void BindShader(GfxShader* shader);
		GP_DLL void BindShader(GfxComputeShader* shader);

		GP_DLL void SetRenderTarget(GfxCubemapRenderTarget* cubemapRT, unsigned int face);
		GP_DLL void SetRenderTarget(GfxRenderTarget* renderTarget);
		GP_DLL void SetDepthStencil(GfxRenderTarget* depthStencil);
		GP_DLL void SetStencilRef(unsigned int ref);

		GP_DLL void Dispatch(unsigned int x = 1, unsigned int y = 1, unsigned int z = 1);
		GP_DLL void Draw(unsigned int numVerts);
		GP_DLL void DrawIndexed(unsigned int numIndices);
		GP_DLL void DrawFullSceen();

		GP_DLL void BeginPass(const std::string& debugName);
		GP_DLL void EndPass();

		void EndFrame();

		inline bool IsInitialized() const { return m_Initialized; }

		inline ShaderFactory* GetShaderFactory() const { return m_ShaderFactory; }

		inline ID3D11Device1* GetDevice() const { return m_Device; }
		inline ID3D11DeviceContext1* GetDeviceContext() { return std::this_thread::get_id() == m_GraphicsThreadID ? m_DeviceContext : m_DeferredContext[m_CurrentDeferredContext]; }

		inline GfxDeviceState* GetState() const { return m_State; }
		inline GfxRenderTarget* GetRenderTarget() const { return m_RenderTarget; }
		inline GfxRenderTarget* GetDepthStencil() const { return m_DepthStencil; }
		inline GfxRenderTarget* GetFinalRT() const { return m_FinalRT; }

	private:
		bool CreateDevice();
#ifdef DEBUG
		void InitDebugLayer();
#endif
		void CreateSwapChain();
		void InitContext();
		void InitSamplers();

		void ClearPipeline();

	private:
		bool m_Initialized = false;

		ShaderFactory* m_ShaderFactory;

		std::thread::id m_GraphicsThreadID;
		ID3D11Device1* m_Device;
		ID3D11DeviceContext1* m_DeviceContext;
		IDXGISwapChain1* m_SwapChain;
		
		unsigned int m_CurrentDeferredContext = 0;
		ID3D11DeviceContext1* m_DeferredContext[3];

		GfxDeviceState m_DefaultState;
		GfxRenderTarget* m_FinalRT;

		unsigned int m_StencilRef = 0xff;
		GfxDeviceState* m_State = &m_DefaultState;
		GfxInputAssembler m_InputAssember;
		GfxRenderTarget* m_RenderTarget = nullptr;
		GfxRenderTarget* m_DepthStencil = nullptr;
		GfxShader* m_Shader = nullptr;
		GfxComputeShader* m_CShader = nullptr;

		unsigned int m_MaxCustomSamplers;
		std::vector<GfxSampler*> m_Samplers;

#ifdef DEBUG
		ID3DUserDefinedAnnotation* m_DebugMarkers;
#endif
	};

	extern GfxDevice* g_Device;
}