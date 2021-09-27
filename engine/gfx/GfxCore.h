#pragma once

#include <vector>

#include "gfx/GfxCommon.h"

struct ID3D11Device1;
struct ID3D11DeviceContext1;
struct IDXGISwapChain1;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState1;
struct ID3D11BlendState1;
struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11ComputeShader;
struct ID3D11InputLayout;
struct ID3D11Texture2D;
#ifdef DEBUG
struct ID3DUserDefinedAnnotation;
#endif

namespace GP
{
	class GfxShader;
	class GfxComputeShader;
	class GfxBufferResource;
	class GfxVertexBuffer;
	class GfxIndexBuffer;
	template<typename T> class GfxConstantBuffer;
	template<typename T> class GfxStructuredBuffer;
	class GfxTexture;
	class GfxRenderTarget;
	class GfxCubemapRenderTarget;
	class GfxDevice;
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

	enum class TextureType
	{
		Texture2D,
		Cubemap
	};

	enum class TextureFormat
	{
		RGBA8_UNORM,
		RGBA_FLOAT
	};

	struct TextureDesc
	{
		std::vector<const void*> texData;
		int width;
		int height;
		TextureType type = TextureType::Texture2D;
		TextureFormat format = TextureFormat::RGBA8_UNORM;
	};

	struct RenderTargetDesc
	{
		unsigned int numRTs = 1;
		unsigned int width;
		unsigned int height;
		bool useDepth = false;
		bool useStencil = false;
	};

	///////////////////////////////////////
	//			Defaults				//
	/////////////////////////////////////

	namespace GfxDefaults
	{
		ENGINE_DLL extern GfxVertexBuffer* VB_CUBE;
		ENGINE_DLL extern GfxVertexBuffer* VB_2DQUAD;
		ENGINE_DLL extern GfxVertexBuffer* VB_QUAD;

		void InitDefaults();
		void DestroyDefaults();
	}

	///////////////////////////////////////
	//			CORE					//
	/////////////////////////////////////

	class GfxDeviceState
	{
		DELETE_COPY_CONSTRUCTOR(GfxDeviceState);
	public:
		ENGINE_DLL GfxDeviceState() {}
		ENGINE_DLL ~GfxDeviceState();

		inline void EnableBackfaceCulling(bool value) { m_BackfaceCullingEnabled = value; }
		inline void EnableDepthTest(bool value) { m_DepthEnabled = value; }
		inline void EnableDepthWrite(bool value) { m_DepthWriteEnabled = value; }
		inline void SetDepthCompareOp(CompareOp depthCompareOp) { m_DepthCompareOp = depthCompareOp; }
		inline void EnableStencil(bool value) { m_StencilEnabled = value; }
		inline void SetStencilReadMask(unsigned int readMask) { m_StencilRead = readMask; }
		inline void SetStencilWriteMask(unsigned int writeMask) { m_StencilWrite = writeMask; }
		inline void SetStencilOp(StencilOp fail, StencilOp depthFail, StencilOp pass) { m_StencilOp[0] = fail; m_StencilOp[1] = depthFail; m_StencilOp[2] = pass; }
		inline void SetStencilCompareOp(CompareOp stencilCompareOp) { m_StencilCompareOp = stencilCompareOp; }
		inline void EnableAlphaBlend(bool value) { m_AlphaBlendEnabled = value; }

		ENGINE_DLL void Compile();

		inline ID3D11DepthStencilState* GetDepthStencilState() const { return m_DepthStencilState; }
		inline ID3D11RasterizerState1* GetRasterizerState() const { return m_RasterizerState; }
		inline ID3D11BlendState1* GetBlendState() const { return m_BlendState; }

		inline bool IsCompiled() const { return m_Compiled; }

	private:
		// Rasterizer state
		bool m_BackfaceCullingEnabled = true;

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

	private:
		bool m_Compiled = false;
		ID3D11DepthStencilState* m_DepthStencilState = nullptr;
		ID3D11RasterizerState1* m_RasterizerState = nullptr;
		ID3D11BlendState1* m_BlendState = nullptr;
	};

	class GfxDevice
	{
		DELETE_COPY_CONSTRUCTOR(GfxDevice);
	public:
		GfxDevice();
		void Init();
		~GfxDevice();

		ENGINE_DLL void Clear(const Vec4& color = VEC4_ZERO);
		ENGINE_DLL void BindState(GfxDeviceState* state);
		ENGINE_DLL void BindIndexBuffer(GfxIndexBuffer* indexBuffer);
		ENGINE_DLL void BindVertexBuffer(GfxVertexBuffer* vertexBuffer);
		template<typename T> void BindConstantBuffer(unsigned int shaderStage, GfxConstantBuffer<T>* constantBuffer, unsigned int binding);
		template<typename T> void BindStructuredBuffer(unsigned int shaderStage, GfxStructuredBuffer<T>* structuredBuffer, unsigned int binding);
		ENGINE_DLL void BindTexture(unsigned int shaderStage, GfxTexture* texture, unsigned int binding);
		ENGINE_DLL void BindTexture(unsigned int shaderStage, GfxRenderTarget* renderTarget, unsigned int binding, unsigned int texIndex = 0);
		ENGINE_DLL void BindTexture(unsigned int shaderStage, GfxCubemapRenderTarget* cubemapRT, unsigned int binding);
		ENGINE_DLL void UnbindTexture(unsigned int shaderStage, unsigned int binding);
		ENGINE_DLL void BindShader(GfxShader* shader);
		ENGINE_DLL void BindShader(GfxComputeShader* shader);

		ENGINE_DLL void SetRenderTarget(GfxCubemapRenderTarget* cubemapRT, unsigned int face);
		ENGINE_DLL void SetRenderTarget(GfxRenderTarget* renderTarget);
		ENGINE_DLL void SetDepthStencil(GfxRenderTarget* depthStencil);
		ENGINE_DLL void SetStencilRef(unsigned int ref);

		ENGINE_DLL void Dispatch(unsigned int x = 1, unsigned int y = 1, unsigned int z = 1);
		ENGINE_DLL void Draw(unsigned int numVerts);
		ENGINE_DLL void DrawIndexed(unsigned int numIndices);
		ENGINE_DLL void DrawFullSceen();

		ENGINE_DLL void BeginPass(const std::string& debugName);
		ENGINE_DLL void EndPass();

		void Present();

		inline bool IsInitialized() const { return m_Initialized; }

		inline ShaderFactory* GetShaderFactory() const { return m_ShaderFactory; }

		inline ID3D11Device1* GetDevice() const { return m_Device; }
		inline ID3D11DeviceContext1* GetDeviceContext() { return m_DeviceContext; }

		inline GfxDeviceState* GetState() const { return m_State; }
		inline GfxRenderTarget* GetRenderTarget() const { return m_RenderTarget; }
		inline GfxRenderTarget* GetDepthStencil() const { return m_DepthStencil; }
		inline GfxRenderTarget* GetFinalRT() const { return m_FinalRT; }

	private:
		ENGINE_DLL void BindConstantBuffer(unsigned int shaderStage, GfxBufferResource* bufferResource, unsigned int binding);
		ENGINE_DLL void BindStructuredBuffer(unsigned int shaderStage, GfxBufferResource* bufferResource, unsigned int binding);

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

		ID3D11Device1* m_Device;
		ID3D11DeviceContext1* m_DeviceContext;
		IDXGISwapChain1* m_SwapChain;

		GfxDeviceState m_DefaultState;
		GfxRenderTarget* m_FinalRT;

		unsigned int m_StencilRef = 0xff;
		GfxDeviceState* m_State = &m_DefaultState;
		GfxRenderTarget* m_RenderTarget = nullptr;
		GfxRenderTarget* m_DepthStencil = nullptr;

		std::vector<ID3D11SamplerState*> m_Samplers;

#ifdef DEBUG
		ID3DUserDefinedAnnotation* m_DebugMarkers;
#endif
	};

	class GfxShader
	{
		const std::string DEFAULT_VS_ENTRY = "vs_main";
		const std::string DEFAULT_PS_ENTRY = "ps_main";

		DELETE_COPY_CONSTRUCTOR(GfxShader);
	public:
		ENGINE_DLL GfxShader(const std::string& path, bool skipPS = false);
		ENGINE_DLL GfxShader(const std::string& path, const std::string& vsEntry, const std::string& psEntry, bool skipPS = false);
		ENGINE_DLL ~GfxShader();

		ENGINE_DLL void Reload();
		inline bool IsInitialized() const { return m_Initialized; }

		inline ID3D11VertexShader* GetVertexShader() const { return m_VertexShader; }
		inline ID3D11PixelShader* GetPixelShader() const { return m_PixelShader; }
		inline ID3D11InputLayout* GetInputLayout() const { return m_InputLayout; }

	private:
		bool CompileShader(const std::string& path, const std::string& vsEntry, const std::string psEntry);

	private:
		bool m_Initialized = false;

		ID3D11VertexShader* m_VertexShader;
		ID3D11PixelShader* m_PixelShader = nullptr;
		ID3D11InputLayout* m_InputLayout;

#ifdef DEBUG
		std::string m_Path;
		std::string m_VSEntry = DEFAULT_VS_ENTRY;
		std::string m_PSEntry = DEFAULT_PS_ENTRY;
#endif // DEBUG
	};

	class GfxComputeShader
	{
		const std::string DEFAULT_ENTRY = "cs_main";

		DELETE_COPY_CONSTRUCTOR(GfxComputeShader);
	public:
		ENGINE_DLL GfxComputeShader(const std::string& path);
		ENGINE_DLL GfxComputeShader(const std::string& path, const std::string& entryPoint);
		ENGINE_DLL ~GfxComputeShader();

		ENGINE_DLL void Reload();
		inline bool IsInitialized() const { return m_Initialized; }

		inline ID3D11ComputeShader* GetShader() const { return m_Shader; }

	private:
		bool CompileShader(const std::string& path, const std::string& entry);

	private:
		bool m_Initialized = false;
		ID3D11ComputeShader* m_Shader;

#ifdef DEBUG
		std::string m_Path;
		std::string m_Entry = DEFAULT_ENTRY;
#endif // DEBUG
	};

	class GfxTexture
	{
		DELETE_COPY_CONSTRUCTOR(GfxTexture);
	public:
		ENGINE_DLL GfxTexture(const TextureDesc& desc);
		ENGINE_DLL ~GfxTexture();

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }

		inline ID3D11ShaderResourceView* GetTextureView() const { return m_TextureView; }

	private:
		unsigned int m_Width;
		unsigned int m_Height;

		ID3D11Texture2D* m_Texture;
		ID3D11ShaderResourceView* m_TextureView;
	};

	class GfxRenderTarget
	{
		DELETE_COPY_CONSTRUCTOR(GfxRenderTarget);
	public:
		static GfxRenderTarget* CreateFromSwapChain(IDXGISwapChain1* swapchain);

	private:
		GfxRenderTarget() {}

	public:
		ENGINE_DLL GfxRenderTarget(const RenderTargetDesc& desc);
		ENGINE_DLL ~GfxRenderTarget();

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }

		inline unsigned int GetNumRTs() const { return m_NumRTs; }
		inline ID3D11RenderTargetView** GetRTViews() const { return (ID3D11RenderTargetView**)m_RTViews.data(); }
		inline ID3D11RenderTargetView* GetRTView(unsigned int index) const { return m_RTViews[index]; }
		inline ID3D11ShaderResourceView* GetSRView(unsigned int index) const { return m_SRViews[index]; }
		inline ID3D11DepthStencilView* GetDSView() const { return m_DSView; }
		inline ID3D11ShaderResourceView* GetDSSRView() const { return m_DSSRView; }

	private:
		void CreateRenderTargets(ID3D11Device1* device, const RenderTargetDesc& desc);
		void CreateDepthStencil(ID3D11Device1* device, const RenderTargetDesc& desc);

	private:
		unsigned int m_Width;
		unsigned int m_Height;

		unsigned int m_NumRTs = 1;
		ID3D11Texture2D* m_TextureMap = nullptr;
		std::vector<ID3D11RenderTargetView*> m_RTViews;
		std::vector<ID3D11ShaderResourceView*> m_SRViews;
		ID3D11DepthStencilView* m_DSView = nullptr;
		ID3D11ShaderResourceView* m_DSSRView = nullptr;
	};

	class GfxCubemapRenderTarget
	{
		DELETE_COPY_CONSTRUCTOR(GfxCubemapRenderTarget);
	public:
		ENGINE_DLL GfxCubemapRenderTarget(const RenderTargetDesc& desc);
		ENGINE_DLL ~GfxCubemapRenderTarget();

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }

		inline ID3D11RenderTargetView* GetRTView(unsigned int face) const { return m_RTViews[face]; }
		inline ID3D11ShaderResourceView* GetSRView() const { return m_SRView; }

	private:
		unsigned int m_Width;
		unsigned int m_Height;

		ID3D11Texture2D* m_TextureMap = nullptr;
		std::vector<ID3D11RenderTargetView*> m_RTViews;
		ID3D11ShaderResourceView* m_SRView = nullptr;
	};

	///////////////////////////////////////
	//			Scoped operations		//
	/////////////////////////////////////

	class BeginRenderPassScoped
	{
	public:
		ENGINE_DLL BeginRenderPassScoped(const std::string& debugName);
		ENGINE_DLL ~BeginRenderPassScoped();
	};

	class DeviceStateScoped
	{
	public:
		ENGINE_DLL DeviceStateScoped(GfxDeviceState* state);
		ENGINE_DLL ~DeviceStateScoped();

	private:
		GfxDeviceState* m_LastState;
	};

	class RenderTargetScoped
	{
	public:
		ENGINE_DLL RenderTargetScoped(GfxRenderTarget* rt, GfxRenderTarget* ds = nullptr);
		ENGINE_DLL ~RenderTargetScoped();

	private:
		GfxRenderTarget* m_LastRT;
		GfxRenderTarget* m_LastDS;
	};
}

#include "GfxCore.hpp"