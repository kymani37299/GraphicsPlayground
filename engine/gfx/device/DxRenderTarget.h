#pragma once

#include <vector>

struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11Device1;
struct IDXGISwapChain1;
struct ID3D11Texture2D;

class DxConverter;
class DxDevice;

struct RenderTargetDesc
{
	unsigned int numRTs = 1;
	unsigned int width;
	unsigned int height;
	bool useDepth = false;
	bool useStencil = false;
};

class DxRenderTarget
{
public:
	static DxRenderTarget* CreateFromSwapChain(DxDevice* device, IDXGISwapChain1* swapchain);

private:
	DxRenderTarget() {}

public:
	DxRenderTarget(DxDevice* device, const RenderTargetDesc& desc);
	~DxRenderTarget();

	inline unsigned int GetWidth() const { return m_Width; }
	inline unsigned int GetHeight() const { return m_Height; }

	inline unsigned int GetNumRTs() const { return m_NumRTs; }
	inline ID3D11RenderTargetView** GetRTViews() const { return (ID3D11RenderTargetView **) m_RTViews.data(); }
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

class DxCubemapRenderTarget
{
	friend class DxConverter;

public:
	DxCubemapRenderTarget(DxDevice* device, const RenderTargetDesc& desc);
	~DxCubemapRenderTarget();

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