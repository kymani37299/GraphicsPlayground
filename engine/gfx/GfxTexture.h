#pragma once

#include "gfx/GfxCommon.h"

#include <string>

struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
struct IDXGISwapChain1;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11Device1;

namespace GP
{
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

	class GfxTexture2D
	{
		DELETE_COPY_CONSTRUCTOR(GfxTexture2D);
	public:
		ENGINE_DLL GfxTexture2D(const std::string& path, unsigned int numMips = 0);
		ENGINE_DLL ~GfxTexture2D();

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }
		inline unsigned int GetBPP() const { return 4; }
		inline unsigned int GetNumMips() const { return m_NumMips; }
		inline TextureFormat GetFormat() const { return m_Format; }

		inline ID3D11ShaderResourceView* GetTextureView() const { return m_TextureView; }

	private:
		unsigned int m_Width;
		unsigned int m_Height;
		unsigned int m_NumMips;
		TextureFormat m_Format;

		ID3D11Texture2D* m_Texture;
		ID3D11ShaderResourceView* m_TextureView;
	};

	class GfxCubemap
	{
		DELETE_COPY_CONSTRUCTOR(GfxCubemap);
	public:
		
		// The order of textures:  Right, Left, Up, Down, Back, Front
		ENGINE_DLL GfxCubemap(std::string textures[6], unsigned int numMips = 0);

		ENGINE_DLL ~GfxCubemap();

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }
		inline unsigned int GetBPP() const { return 4; }
		inline unsigned int GetNumMips() const { return m_NumMips; }
		inline TextureFormat GetFormat() const { return m_Format; }

		inline ID3D11ShaderResourceView* GetTextureView() const { return m_TextureView; }

	private:
		unsigned int m_Width;
		unsigned int m_Height;
		unsigned int m_NumMips;
		TextureFormat m_Format;

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
}