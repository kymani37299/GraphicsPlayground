#pragma once

#include "gfx/GfxCommon.h"

#include <string>
#include <vector>

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
		RGBA_FLOAT,
		R24G8_TYPELESS,
		R32_TYPELESS,
	};

	enum TextureResourceCreationFlags
	{
		TRCF_BindSRV = 1 << 0,
		TRCF_BindRT = 1 << 1,
		TRCF_BindDepthStencil = 1 << 2,
		TRCF_BindCubemap = 1 << 3,

		TRCF_Static = 1 << 4,
		TRCF_Dynamic = 1 << 5,
		TRCF_Staging = 1 << 6,

		TRCF_CPURead = 1 << 7,
		TRCF_CPUWrite = 1 << 8,
		TRCF_CPUReadWrite = TRCF_CPURead | TRCF_CPUWrite,

		TRCF_GenerateMips = 1 << 9
	};

	static unsigned int MAX_MIPS = 0;

	class TextureResource2D
	{
	public:
		TextureResource2D(ID3D11Texture2D* resource, unsigned int width, unsigned int height, TextureFormat format, unsigned int numMips, unsigned int arraySize, unsigned int creationFlags) :
			TextureResource2D(width, height, format, numMips, arraySize, creationFlags)
		{
			m_Resource = resource;
		}

		TextureResource2D(unsigned int width, unsigned int height, TextureFormat format, unsigned int numMips, unsigned int arraySize, unsigned int creationFlags):
			m_Width(width),
			m_Height(height),
			m_Format(format),
			m_NumMips(numMips),
			m_ArraySize(arraySize),
			m_CreationFlags(creationFlags),
			m_RefCount(1)
		{
			ASSERT(!(creationFlags & TRCF_BindCubemap) || arraySize == 6, "[TextureResource2D] ArraySize must be 6 when BindCubemap flag is enabled.");
			ASSERT(numMips == 1 || creationFlags & TRCF_GenerateMips, "[TextureResource2D] Creating textrure resource with numMips > 1 but flag for Mip generation is off.");
		}

		void Initialize();
		void InitializeWithData(void* data[]);

		void Upload(void* data, unsigned int arrayIndex);

		inline bool IsInitialized() const { return m_Resource != nullptr; }
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
		ID3D11Texture2D* m_Resource = nullptr;
		unsigned int m_RefCount;

		unsigned int m_Width;
		unsigned int m_Height;
		TextureFormat m_Format;
		unsigned int m_NumMips;
		unsigned int m_ArraySize;
		unsigned int m_CreationFlags;

		unsigned int m_RowPitch;
		unsigned int m_SlicePitch;
	};

	class GfxTexture2D
	{
		DELETE_COPY_CONSTRUCTOR(GfxTexture2D);
	public:
		ENGINE_DLL GfxTexture2D(const std::string& path, unsigned int numMips = 1);
		ENGINE_DLL GfxTexture2D(TextureResource2D* textureResource, unsigned int arrayIndex = 0);
		ENGINE_DLL GfxTexture2D(unsigned int width, unsigned int height, unsigned int numMips = 1);
		ENGINE_DLL ~GfxTexture2D();

		inline TextureResource2D* GetResource() const { return m_Resource; }
		inline ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }
		inline void Upload(void* data)
		{
			m_Resource->Upload(data, 0);
		}

	private:
		TextureResource2D* m_Resource;
		ID3D11ShaderResourceView* m_SRV;
	};

	class GfxCubemap
	{
		DELETE_COPY_CONSTRUCTOR(GfxCubemap);
	public:
		
		// The order of textures:  Right, Left, Up, Down, Back, Front
		ENGINE_DLL GfxCubemap(std::string textures[6], unsigned int numMips = 1);
		ENGINE_DLL GfxCubemap(TextureResource2D* textureResource);
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
		ENGINE_DLL GfxRenderTarget(unsigned int width, unsigned int height, unsigned int numRTs = 1, bool useDepth = false, bool useStencil = false);
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
		unsigned int m_NumRTs = 1;

		std::vector<TextureResource2D*> m_Resources;
		std::vector<ID3D11RenderTargetView*> m_RTVs;
		ID3D11DepthStencilView* m_DSV = nullptr;
		TextureResource2D* m_DepthResource = nullptr;
	};

	class GfxCubemapRenderTarget
	{
		DELETE_COPY_CONSTRUCTOR(GfxCubemapRenderTarget);
	public:
		ENGINE_DLL GfxCubemapRenderTarget(unsigned int width, unsigned int height);
		ENGINE_DLL ~GfxCubemapRenderTarget();

		inline unsigned int GetWidth() const { return m_Resource->GetWidth(); }
		inline unsigned int GetHeight() const { return m_Resource->GetHeight(); }

		inline ID3D11RenderTargetView* GetRTV(unsigned int face) const { return m_RTVs[face]; }

	private:

		TextureResource2D* m_Resource;
		ID3D11RenderTargetView* m_RTVs[6];
	};
}