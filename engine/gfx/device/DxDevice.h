#pragma once

#include "core/Core.h"

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
#ifdef DEBUG
struct ID3DUserDefinedAnnotation;
#endif

class DxShader;
class DxVertexBuffer;
class DxIndexBuffer;
template<typename T> class DxConstantBuffer;
template<typename T> class DxStructuredBuffer;
class DxTexture;
class DxRenderTarget;
class DxCubemapRenderTarget;
class DxDevice;

class Window;

enum ShaderStage
{
	VS = 1,
	GS = 2,
	PS = 4,
	// 8
};

enum StencilOp
{
	Discard,
	Keep,
	Replace
};

enum CompareOp
{
	Always,
	Equals,
	Less
};

class DxDeviceState
{
public:
	~DxDeviceState();

	inline void EnableBackfaceCulling(bool value) { m_BackfaceCullingEnabled = value; }
	inline void EnableDepthTest(bool value) { m_DepthEnabled = value; }
	inline void EnableDepthWrite(bool value) { m_DepthWriteEnabled = value; }
	inline void SetDepthCompareOp(CompareOp depthCompareOp) { m_DepthCompareOp = depthCompareOp; }
	inline void EnableStencil(bool value) { m_StencilEnabled = value; }
	inline void SetStencilReadMask(unsigned int readMask) { m_StencilRead = readMask; }
	inline void SetStencilWriteMask(unsigned int writeMask) { m_StencilWrite = writeMask; }
	inline void SetStencilOp(StencilOp fail, StencilOp depthFail, StencilOp pass) { m_StencilOp[0] = fail;m_StencilOp[1] = depthFail; m_StencilOp[2] = pass; }
	inline void SetStencilCompareOp(CompareOp stencilCompareOp) { m_StencilCompareOp = stencilCompareOp; }
	inline void EnableAlphaBlend(bool value) { m_AlphaBlendEnabled = value; }

	void Compile(DxDevice* device);

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
	CompareOp m_DepthCompareOp = Less;
	bool m_StencilEnabled = false;
	unsigned int m_StencilRead = 0xff;
	unsigned int m_StencilWrite = 0xff;
	StencilOp m_StencilOp[3] = { Keep, Keep, Keep };
	CompareOp m_StencilCompareOp = Always;

	// Blend state
	bool m_AlphaBlendEnabled = false;

private:
	bool m_Compiled = false;
	ID3D11DepthStencilState* m_DepthStencilState = nullptr;
	ID3D11RasterizerState1* m_RasterizerState = nullptr;
	ID3D11BlendState1* m_BlendState = nullptr;
};

class DxDevice
{
public:
	DxDevice(Window* window);
	~DxDevice();

	void Clear(const Vec4& color = VEC4_ZERO);

	void BindState(DxDeviceState* state);
	void BindIndexBuffer(DxIndexBuffer* indexBuffer);
	void BindVertexBuffer(DxVertexBuffer* vertexBuffer);
	template<typename T> void BindConstantBuffer(ShaderStage stage, DxConstantBuffer<T>* constantBuffer, unsigned int binding);
	template<typename T> void BindStructuredBuffer(ShaderStage stage, DxStructuredBuffer<T>* structuredBuffer, unsigned int binding);
	void BindTexture(ShaderStage stage, DxTexture* texture, unsigned int binding);
	void BindTexture(ShaderStage stage, DxRenderTarget* renderTarget, unsigned int binding, unsigned int texIndex = 0);
	void BindTexture(ShaderStage stage, DxCubemapRenderTarget* cubemapRT, unsigned int binding);
	void UnbindTexture(ShaderStage stage, unsigned int binding);
	void BindShader(DxShader* shader);
	
	void SetRenderTarget(DxCubemapRenderTarget* cubemapRT, unsigned int face);
	void SetRenderTarget(DxRenderTarget* renderTarget);
	void SetDepthStencil(DxRenderTarget* depthStencil);
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

	inline DxDeviceState* GetState() const { return m_State; }
	inline DxRenderTarget* GetRenderTarget() const { return m_RenderTarget; }
	inline DxRenderTarget* GetDepthStencil() const { return m_DepthStencil; }
	inline DxRenderTarget* GetFinalRT() const { return m_FinalRT; }

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

	DxDeviceState m_DefaultState;
	DxRenderTarget* m_FinalRT;

	unsigned int m_StencilRef = 0xff;
	DxDeviceState* m_State = &m_DefaultState;
	DxRenderTarget* m_RenderTarget = nullptr;
	DxRenderTarget* m_DepthStencil = nullptr;

	ID3D11SamplerState* m_PointBorderSampler;
	ID3D11SamplerState* m_LinearBorderSampler;

#ifdef DEBUG
	ID3DUserDefinedAnnotation* m_DebugMarkers;
#endif
};

#include "DxDevice.hpp"