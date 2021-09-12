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
		~GfxDeviceState();

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

		void Compile(GfxDevice* device);

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
		GfxDevice(Window* window);
		~GfxDevice();

		void Clear(const Vec4& color = VEC4_ZERO);

		void BindState(GfxDeviceState* state);
		void BindIndexBuffer(GfxIndexBuffer* indexBuffer);
		void BindVertexBuffer(GfxVertexBuffer* vertexBuffer);
		template<typename T> void BindConstantBuffer(ShaderStage stage, GfxConstantBuffer<T>* constantBuffer, unsigned int binding);
		template<typename T> void BindStructuredBuffer(ShaderStage stage, GfxStructuredBuffer<T>* structuredBuffer, unsigned int binding);
		void BindTexture(ShaderStage stage, GfxTexture* texture, unsigned int binding);
		void BindTexture(ShaderStage stage, GfxRenderTarget* renderTarget, unsigned int binding, unsigned int texIndex = 0);
		void BindTexture(ShaderStage stage, GfxCubemapRenderTarget* cubemapRT, unsigned int binding);
		void UnbindTexture(ShaderStage stage, unsigned int binding);
		void BindShader(GfxShader* shader);

		void SetRenderTarget(GfxCubemapRenderTarget* cubemapRT, unsigned int face);
		void SetRenderTarget(GfxRenderTarget* renderTarget);
		void SetDepthStencil(GfxRenderTarget* depthStencil);
		void SetStencilRef(unsigned int ref);

		void Draw(unsigned int numVerts);
		void DrawIndexed(unsigned int numIndices);
		void DrawFullSceen();

		void BeginPass(const std::string& debugName);
		void EndPass();

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
		GfxShader(GfxDevice* device, const ShaderDesc& desc);
		~GfxShader();

		void Reload(GfxDevice* device);
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
		friend class GfxBufferAllocator;

	public:
		GfxVertexBuffer(GfxDevice* device, const VertexBufferData& data);
		~GfxVertexBuffer();

		inline unsigned int GetStride() const { return m_Stride; }
		inline unsigned int GetOffset() const { return m_Offset; }
		inline unsigned int GetNumVerts() const { return m_NumVerts; }
		inline ID3D11Buffer* GetBuffer() const { return m_Buffer; }

	private:
		GfxVertexBuffer() {}

	private:
		unsigned int m_Stride = 0;
		unsigned int m_Offset = 0;
		unsigned int m_NumVerts = 0;

		bool m_BufferOwner = true;
		ID3D11Buffer* m_Buffer = nullptr;
	};

	class GfxIndexBuffer
	{
		friend class GfxBufferAllocator;

	public:
		GfxIndexBuffer(GfxDevice* device, unsigned int* pIndices, unsigned int numIndices);
		~GfxIndexBuffer();

		inline ID3D11Buffer* GetBuffer() const { return m_Buffer; }
		inline unsigned int GetOffset() const { return m_Offset; }
		inline unsigned int GetNumIndices() const { return m_NumIndices; }

	private:
		GfxIndexBuffer() {}

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
		GfxConstantBuffer(GfxDevice* device);
		~GfxConstantBuffer();

		void Upload(const T& data);

		inline ID3D11Buffer* GetBuffer() const { return m_Buffer; }
	private:
		unsigned int m_Size;

		GfxDevice* m_Device;

		ID3D11Buffer* m_Buffer;
	};

	template<typename T>
	class GfxStructuredBuffer
	{
	public:
		GfxStructuredBuffer(GfxDevice* device, unsigned int numElements);
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

	class GfxBufferAllocator
	{
		static constexpr unsigned int MAX_BUFFER_SIZE = 300 * 1024 * 1024; // 300MB
	public:
		static GfxBufferAllocator* s_Instance;
		static void Init(GfxDevice* device) { ASSERT(!s_Instance, "[GfxBufferAllocator::Init] s_Instance != null"); s_Instance = new GfxBufferAllocator(device); }
		static void Deinit() { SAFE_DELETE(s_Instance); }
		static GfxBufferAllocator* Get() { return s_Instance; }

	private:
		GfxBufferAllocator(GfxDevice* device);
		~GfxBufferAllocator();

	public:
		GfxVertexBuffer* AllocateVertexBuffer(GfxDevice* device, const VertexBufferData& data);
		GfxIndexBuffer* AllocateIndexBuffer(GfxDevice* device, unsigned int* pIndices, unsigned int numIndices);

	private:
		unsigned int m_VertexFilledSize = 0;
		ID3D11Buffer* m_VertexBuffer;

		unsigned int m_IndexFilledSize = 0;
		ID3D11Buffer* m_IndexBuffer;
	};

	class GfxTexture
	{
	public:
		GfxTexture(GfxDevice* device, const TextureDesc& desc);
		~GfxTexture();

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
		static GfxRenderTarget* CreateFromSwapChain(GfxDevice* device, IDXGISwapChain1* swapchain);

	private:
		GfxRenderTarget() {}

	public:
		GfxRenderTarget(GfxDevice* device, const RenderTargetDesc& desc);
		~GfxRenderTarget();

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
		GfxCubemapRenderTarget(GfxDevice* device, const RenderTargetDesc& desc);
		~GfxCubemapRenderTarget();

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
		BeginRenderPassScoped(GfxDevice& device, const std::string& debugName) :
			m_Device(device)
		{
			m_Device.BeginPass(debugName);
		}

		~BeginRenderPassScoped()
		{
			m_Device.EndPass();
		}

	private:
		GfxDevice& m_Device;
	};

	class DeviceStateScoped
	{
	public:
		DeviceStateScoped(GfxDevice& device, GfxDeviceState* state) :
			m_Device(device),
			m_LastState(device.GetState())
		{
			m_Device.BindState(state);
		}

		~DeviceStateScoped()
		{
			m_Device.BindState(m_LastState);
		}

	private:
		GfxDevice& m_Device;
		GfxDeviceState* m_LastState;
	};

	class RenderTargetScoped
	{
	public:
		RenderTargetScoped(GfxDevice& device, GfxRenderTarget* rt, GfxRenderTarget* ds = nullptr) :
			m_Device(device),
			m_LastRT(device.GetRenderTarget()),
			m_LastDS(device.GetDepthStencil())
		{
			m_Device.SetRenderTarget(rt);
			m_Device.SetDepthStencil(ds);
		}

		~RenderTargetScoped()
		{
			m_Device.SetRenderTarget(m_LastRT);
			m_Device.SetDepthStencil(m_LastDS);
		}

	private:
		GfxDevice& m_Device;
		GfxRenderTarget* m_LastRT;
		GfxRenderTarget* m_LastDS;
	};
}