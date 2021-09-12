#pragma once

#include "core/Core.h"

#include <vector>

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
struct ID3D11InputLayout;
struct ID3D11Texture2D;
#ifdef DEBUG
struct ID3DUserDefinedAnnotation;
#endif

namespace GP
{
	class Window;

	class GfxShader;
	class GfxVertexBuffer;
	class GfxIndexBuffer;
	template<typename T> class GfxConstantBuffer;
	template<typename T> class GfxStructuredBuffer;
	class GfxTexture;
	class GfxRenderTarget;
	class GfxCubemapRenderTarget;
	class GfxDevice;

	///////////////////////////////////////
	//			MODEL					//
	/////////////////////////////////////

	enum ShaderStage
	{
		VS = 1,
		GS = 2,
		PS = 4,
		// 8
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

	enum class ShaderInputFormat
	{
		Float,
		Float2,
		Float3,
		Float4
	};

	struct ShaderInput
	{
		ShaderInputFormat format;
		const char* semanticName;
	};

	struct ShaderDesc
	{
		std::string path;
		std::vector<ShaderInput> inputs;
		bool skipPS = false;
	};

	struct VertexBufferData
	{
		void* pData;
		unsigned int numBytes;
		unsigned int stride;
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
	//			CORE					//
	/////////////////////////////////////

	class GfxDeviceState
	{
	public:
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
	public:
		GfxDevice();
		void Init(Window* window);
		~GfxDevice();

		ENGINE_DLL void Clear(const Vec4& color = VEC4_ZERO);
		ENGINE_DLL void BindState(GfxDeviceState* state);
		ENGINE_DLL void BindIndexBuffer(GfxIndexBuffer* indexBuffer);
		ENGINE_DLL void BindVertexBuffer(GfxVertexBuffer* vertexBuffer);
		template<typename T> void BindConstantBuffer(ShaderStage stage, GfxConstantBuffer<T>* constantBuffer, unsigned int binding);
		template<typename T> void BindStructuredBuffer(ShaderStage stage, GfxStructuredBuffer<T>* structuredBuffer, unsigned int binding);
		ENGINE_DLL void BindTexture(ShaderStage stage, GfxTexture* texture, unsigned int binding);
		ENGINE_DLL void BindTexture(ShaderStage stage, GfxRenderTarget* renderTarget, unsigned int binding, unsigned int texIndex = 0);
		ENGINE_DLL void BindTexture(ShaderStage stage, GfxCubemapRenderTarget* cubemapRT, unsigned int binding);
		ENGINE_DLL void UnbindTexture(ShaderStage stage, unsigned int binding);
		ENGINE_DLL void BindShader(GfxShader* shader);

		ENGINE_DLL void SetRenderTarget(GfxCubemapRenderTarget* cubemapRT, unsigned int face);
		ENGINE_DLL void SetRenderTarget(GfxRenderTarget* renderTarget);
		ENGINE_DLL void SetDepthStencil(GfxRenderTarget* depthStencil);
		ENGINE_DLL void SetStencilRef(unsigned int ref);

		ENGINE_DLL void Draw(unsigned int numVerts);
		ENGINE_DLL void DrawIndexed(unsigned int numIndices);
		ENGINE_DLL void DrawFullSceen();

		ENGINE_DLL void BeginPass(const std::string& debugName);
		ENGINE_DLL void EndPass();

		void Present();

		inline bool IsInitialized() const { return m_Initialized; }

		inline ID3D11Device1* GetDevice() const { return m_Device; }
		inline ID3D11DeviceContext1* GetDeviceContext() { return m_DeviceContext; }

		inline GfxDeviceState* GetState() const { return m_State; }
		inline GfxRenderTarget* GetRenderTarget() const { return m_RenderTarget; }
		inline GfxRenderTarget* GetDepthStencil() const { return m_DepthStencil; }
		inline GfxRenderTarget* GetFinalRT() const { return m_FinalRT; }

	private:
		void BindConstantBuffer(ShaderStage stage, ID3D11Buffer* constantBuffer, unsigned int binding);
		void BindStructuredBuffer(ShaderStage stage, ID3D11ShaderResourceView* structuredBufferSrv, unsigned int binding);

		bool CreateDevice();
#ifdef DEBUG
		void InitDebugLayer();
#endif
		void CreateSwapChain(Window* window);
		void InitContext(Window* window);
		void InitSamplers();

	private:
		bool m_Initialized = false;

		ID3D11Device1* m_Device;
		ID3D11DeviceContext1* m_DeviceContext;
		IDXGISwapChain1* m_SwapChain;

		GfxDeviceState m_DefaultState;
		GfxRenderTarget* m_FinalRT;

		unsigned int m_StencilRef = 0xff;
		GfxDeviceState* m_State = &m_DefaultState;
		GfxRenderTarget* m_RenderTarget = nullptr;
		GfxRenderTarget* m_DepthStencil = nullptr;

		ID3D11SamplerState* m_PointBorderSampler;
		ID3D11SamplerState* m_LinearBorderSampler;

#ifdef DEBUG
		ID3DUserDefinedAnnotation* m_DebugMarkers;
#endif
	};

	class GfxShader
	{
	public:
		ENGINE_DLL GfxShader(const ShaderDesc& desc);
		ENGINE_DLL ~GfxShader();

		ENGINE_DLL void Reload();
		inline bool IsInitialized() const { return m_Initialized; }

		inline ID3D11VertexShader* GetVertexShader() const { return m_VertexShader; }
		inline ID3D11PixelShader* GetPixelShader() const { return m_PixelShader; }
		inline ID3D11InputLayout* GetInputLayout() const { return m_InputLayout; }

	private:
		bool m_Initialized = false;

		ID3D11VertexShader* m_VertexShader;
		ID3D11PixelShader* m_PixelShader = nullptr;
		ID3D11InputLayout* m_InputLayout;

#ifdef DEBUG
	private:
		ShaderDesc m_Desc;
#endif
	};

	class GfxVertexBuffer
	{
	public:
		ENGINE_DLL GfxVertexBuffer(const VertexBufferData& data);
		ENGINE_DLL ~GfxVertexBuffer();

		inline unsigned int GetStride() const { return m_Stride; }
		inline unsigned int GetOffset() const { return m_Offset; }
		inline unsigned int GetNumVerts() const { return m_NumVerts; }
		inline ID3D11Buffer* GetBuffer() const { return m_Buffer; }

	private:
		unsigned int m_Stride = 0;
		unsigned int m_Offset = 0;
		unsigned int m_NumVerts = 0;

		bool m_BufferOwner = true;
		ID3D11Buffer* m_Buffer = nullptr;
	};

	class GfxIndexBuffer
	{
	public:
		ENGINE_DLL GfxIndexBuffer(unsigned int* pIndices, unsigned int numIndices);
		ENGINE_DLL ~GfxIndexBuffer();

		inline ID3D11Buffer* GetBuffer() const { return m_Buffer; }
		inline unsigned int GetOffset() const { return m_Offset; }
		inline unsigned int GetNumIndices() const { return m_NumIndices; }

	private:
		unsigned int m_Offset = 0;
		unsigned int m_NumIndices = 0;

		bool m_BufferOwner = true;
		ID3D11Buffer* m_Buffer = nullptr;
	};

	template<typename T>
	class GfxConstantBuffer
	{
	public:
		GfxConstantBuffer();
		~GfxConstantBuffer();

		void Upload(const T& data);

		inline ID3D11Buffer* GetBuffer() const { return m_Buffer; }
	private:
		unsigned int m_Size;

		ID3D11Buffer* m_Buffer;
	};

	template<typename T>
	class GfxStructuredBuffer
	{
	public:
		GfxStructuredBuffer(unsigned int numElements);
		~GfxStructuredBuffer();

		void Upload(const T& data, unsigned int index);

		inline ID3D11ShaderResourceView* GetSRV() const { return m_Srv; }

	private:
		unsigned int m_NumElements;
		unsigned int m_ElementSize;

		GfxDevice* m_Device;

		ID3D11Buffer* m_Buffer;
		ID3D11ShaderResourceView* m_Srv;
	};

	class GfxTexture
	{
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