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
	enum class TextureFormat
	{
		UNKNOWN,
		RGBA8_UNORM,
		RGBA_FLOAT
	};

	enum RenderTargetCreationFlags
	{
		RTCF_UseDepth = 1 << 0,
		RTCF_UseStencil = 1 << 1,
		RTCF_SRV = 1 << 2
	};

	class TextureResource2D
	{
	public:
		TextureResource2D(ID3D11Texture2D* resource, unsigned int width, unsigned int height, TextureFormat format, unsigned int numMips, unsigned int arraySize):
			m_Resource(resource),
			m_Width(width),
			m_Height(height),
			m_Format(format),
			m_NumMips(numMips),
			m_ArraySize(arraySize),
			m_RefCount(1)
		{ }

		inline ID3D11Texture2D* GetResource() const { return m_Resource; }

		inline void AddRef()
		{
			m_RefCount++;
		}

		inline void Release()
		{
			m_RefCount--;
			if (m_RefCount == 0) delete this;
		}

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }
		inline TextureFormat GetFormat() const { return m_Format; }
		inline unsigned int GetNumMips() const { return m_NumMips; }
		inline unsigned int GetArraySize() const { return m_ArraySize; }

	private:
		~TextureResource2D();

	private:
		ID3D11Texture2D* m_Resource;
		unsigned int m_RefCount;

		unsigned int m_Width;
		unsigned int m_Height;
		TextureFormat m_Format;
		unsigned int m_NumMips;
		unsigned int m_ArraySize;
	};

	class GfxTexture2D
	{
		DELETE_COPY_CONSTRUCTOR(GfxTexture2D);
	public:
		ENGINE_DLL GfxTexture2D(const std::string& path, unsigned int numMips = 0);
		ENGINE_DLL GfxTexture2D(TextureResource2D* textureResource);
		ENGINE_DLL ~GfxTexture2D();

		inline TextureResource2D* GetResource() const { return m_Resource; }
		inline ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }

	private:
		TextureResource2D* m_Resource;
		ID3D11ShaderResourceView* m_SRV;
	};

	class GfxCubemap
	{
		DELETE_COPY_CONSTRUCTOR(GfxCubemap);
	public:
		
		// The order of textures:  Right, Left, Up, Down, Back, Front
		ENGINE_DLL GfxCubemap(std::string textures[6], unsigned int numMips = 0);
		ENGINE_DLL ~GfxCubemap();

		inline TextureResource2D* GetResource() const { return m_Resource; }
		inline ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }

	private:
		TextureResource2D* m_Resource;
		ID3D11ShaderResourceView* m_SRV;
	};

	class GfxRenderTarget
	{
		DELETE_COPY_CONSTRUCTOR(GfxRenderTarget);
	public:
		static GfxRenderTarget* CreateFromSwapChain(IDXGISwapChain1* swapchain);

	private:
		GfxRenderTarget() {}

	public:
		ENGINE_DLL GfxRenderTarget(unsigned int width, unsigned int height, unsigned int numRTs = 1, unsigned int creationFlags = 0);
		ENGINE_DLL ~GfxRenderTarget();

		inline unsigned int GetNumRTs() const { return m_NumRTs; }

		// TODO: Cover when DS only render target
		inline unsigned int GetWidth() const { return m_Resources[0]->GetWidth(); }
		inline unsigned int GetHeight() const { return m_Resources[0]->GetHeight(); }

		inline TextureResource2D* GetResource(unsigned int index = 0) const { return m_Resources[index]; }
		inline ID3D11RenderTargetView** GetRTVs() const { return (ID3D11RenderTargetView**) m_RTVs.data(); }
		inline ID3D11RenderTargetView* GetRTV(unsigned int index) { return m_RTVs[index]; }
		inline ID3D11DepthStencilView* GetDSV() const { return m_DSV; }

	private:
		unsigned int m_CreationFlags;
		unsigned int m_NumRTs = 1;

		std::vector<TextureResource2D*> m_Resources;
		std::vector<ID3D11RenderTargetView*> m_RTVs;
		ID3D11DepthStencilView* m_DSV = nullptr;
		TextureResource2D* m_DepthResource = nullptr;
	};

	/*
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
	}; */
}