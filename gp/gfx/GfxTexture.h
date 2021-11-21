#pragma once

#include "gfx/GfxCommon.h"

#include <string>
#include <vector>

struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11Texture2D;
struct ID3D11Texture3D;
struct IDXGISwapChain1;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11SamplerState;

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
		TRCF_BindUAV = 1 << 1,
		TRCF_BindRT = 1 << 2,
		TRCF_BindDepthStencil = 1 << 3,
		TRCF_BindCubemap = 1 << 4,

		TRCF_Static = 1 << 5,
		TRCF_Dynamic = 1 << 6,
		TRCF_Staging = 1 << 7,

		TRCF_CPURead = 1 << 8,
		TRCF_CPUWrite = 1 << 9,
		TRCF_CPUReadWrite = TRCF_CPURead | TRCF_CPUWrite,

		TRCF_GenerateMips = 1 << 10
	};

	enum class SamplerFilter
	{
		Point,
		Linear,
		Trilinear,
		Anisotropic
	};

	enum class SamplerMode
	{
		Wrap,
		Clamp,
		Border,
		Mirror,
		MirrorOnce
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

		GP_DLL void Upload(void* data, unsigned int arrayIndex);

		inline bool Initialized() const { return m_Resource != nullptr; }
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

	class TextureResource3D
	{
	public:
		TextureResource3D(ID3D11Texture3D* resource, unsigned int width, unsigned int height, unsigned int depth, TextureFormat format, unsigned int numMips, unsigned int creationFlags) :
			TextureResource3D(width, height, depth, format, numMips, creationFlags)
		{
			m_Resource = resource;
		}

		TextureResource3D(unsigned int width, unsigned int height, unsigned int depth, TextureFormat format, unsigned int numMips, unsigned int creationFlags) :
			m_Width(width),
			m_Height(height),
			m_Depth(depth),
			m_Format(format),
			m_NumMips(numMips),
			m_CreationFlags(creationFlags),
			m_RefCount(1)
		{
			ASSERT(numMips == 1 || creationFlags & TRCF_GenerateMips, "[TextureResource3D] Creating textrure resource with numMips > 1 but flag for Mip generation is off.");
		}

		void Initialize();
		void InitializeWithData(void* data[]);

		inline bool Initialized() const { return m_Resource != nullptr; }
		inline ID3D11Texture3D* GetResource() const { return m_Resource; }

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
		inline unsigned int GetDepth() const { return m_Height; }
		inline TextureFormat GetFormat() const { return m_Format; }
		inline unsigned int GetNumMips() const { return m_NumMips; }

	private:
		~TextureResource3D();

	private:
		ID3D11Texture3D* m_Resource = nullptr;
		unsigned int m_RefCount;

		unsigned int m_Width;
		unsigned int m_Height;
		unsigned int m_Depth;
		TextureFormat m_Format;
		unsigned int m_NumMips;
		unsigned int m_CreationFlags;

		unsigned int m_RowPitch;
		unsigned int m_SlicePitch;
	};

	class GfxTexture2D
	{
		DELETE_COPY_CONSTRUCTOR(GfxTexture2D);
	public:
		GP_DLL GfxTexture2D(const std::string& path, unsigned int numMips = 1);
		GP_DLL GfxTexture2D(TextureResource2D* textureResource, unsigned int arrayIndex = 0);
		GP_DLL GfxTexture2D(unsigned int width, unsigned int height, unsigned int numMips = 1);
		GP_DLL ~GfxTexture2D();

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

	class GfxTextureArray2D
	{
		DELETE_COPY_CONSTRUCTOR(GfxTextureArray2D);
	public:
		GP_DLL GfxTextureArray2D(unsigned int width, unsigned int height, unsigned int arraySize, unsigned int numMips = 1);
		GP_DLL ~GfxTextureArray2D();

		inline TextureResource2D* GetResource() const { return m_Resource; }
		inline ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }
		inline void Upload(unsigned int index, void* data)
		{
			m_Resource->Upload(data, index);
		}

		GP_DLL void Upload(const std::string& path, unsigned int index);

	private:
		TextureResource2D* m_Resource;
		ID3D11ShaderResourceView* m_SRV;
	};

	class GfxCubemap
	{
		DELETE_COPY_CONSTRUCTOR(GfxCubemap);
	public:
		
		// The order of textures:  Right, Left, Up, Down, Back, Front
		GP_DLL GfxCubemap(std::string textures[6], unsigned int numMips = 1);
		GP_DLL GfxCubemap(TextureResource2D* textureResource);
		GP_DLL ~GfxCubemap();

		inline TextureResource2D* GetResource() const { return m_Resource; }
		inline ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }

	private:
		TextureResource2D* m_Resource;
		ID3D11ShaderResourceView* m_SRV;
	};

	class GfxBaseTexture3D
	{
	protected:
		GfxBaseTexture3D() {}

		GfxBaseTexture3D(TextureResource3D* resource) :
			m_Resource(resource)
		{
			m_Resource->AddRef();
		}

	public:
		inline TextureResource3D* GetResource() const { return m_Resource; }
		inline ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }
		inline ID3D11UnorderedAccessView* GetUAV() const { return m_UAV; }

	protected:
		TextureResource3D* m_Resource = nullptr;
		ID3D11ShaderResourceView* m_SRV = nullptr;
		ID3D11UnorderedAccessView* m_UAV = nullptr;

	};

	class GfxTexture3D : public GfxBaseTexture3D
	{
		DELETE_COPY_CONSTRUCTOR(GfxTexture3D);
	public:
		GP_DLL GfxTexture3D(unsigned int width, unsigned int height, unsigned int depth, unsigned int numMips = 1, unsigned int creationFlags = 0);
		GP_DLL GfxTexture3D(const GfxBaseTexture3D& resource);
		GP_DLL ~GfxTexture3D();
	};

	class GfxRWTexture3D : public GfxBaseTexture3D
	{
		DELETE_COPY_CONSTRUCTOR(GfxRWTexture3D);
	public:
		GP_DLL GfxRWTexture3D(unsigned int width, unsigned int height, unsigned int depth, unsigned int numMips = 1, unsigned int creationFlags = 0);
		GP_DLL GfxRWTexture3D(const GfxBaseTexture3D& resource);
		GP_DLL ~GfxRWTexture3D();
	};

	class GfxRenderTarget
	{
		DELETE_COPY_CONSTRUCTOR(GfxRenderTarget);
	public:
		static GfxRenderTarget* CreateFromSwapChain(IDXGISwapChain1* swapchain);

	private:
		GfxRenderTarget() {}

	public:
		GP_DLL GfxRenderTarget(unsigned int width, unsigned int height, unsigned int numRTs = 1, bool useDepth = false, bool useStencil = false);
		GP_DLL ~GfxRenderTarget();

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
		GP_DLL GfxCubemapRenderTarget(unsigned int width, unsigned int height);
		GP_DLL ~GfxCubemapRenderTarget();

		inline unsigned int GetWidth() const { return m_Resource->GetWidth(); }
		inline unsigned int GetHeight() const { return m_Resource->GetHeight(); }

		inline ID3D11RenderTargetView* GetRTV(unsigned int face) const { return m_RTVs[face]; }

	private:

		TextureResource2D* m_Resource;
		ID3D11RenderTargetView* m_RTVs[6];
	};

	class GfxSampler
	{
		static constexpr float MAX_MIP_VALUE = 3.402823466e+38f;
		const Vec4 BLACK_BORDER{ 0.0f, 0.0f, 0.0f, 1.0f };
	public:
		inline GfxSampler(SamplerFilter filter, SamplerMode mode) :
			GfxSampler(filter, mode, BLACK_BORDER, 0, 0, GetDefaultMaxMip(filter), GetDefaultMaxAnisotropy(filter)) {}
		inline GfxSampler(SamplerFilter filter, SamplerMode mode, Vec4 borderColor) :
			GfxSampler(filter, mode, borderColor, 0, 0, GetDefaultMaxMip(filter), GetDefaultMaxAnisotropy(filter)) {}
		inline GfxSampler(SamplerFilter filter, SamplerMode mode, float minMip, float maxMip, float mipBias = 0.0f, unsigned int maxAnisotropy = 0) :
			GfxSampler(filter, mode, BLACK_BORDER, mipBias, minMip, maxMip, maxAnisotropy) {}

		GP_DLL GfxSampler(SamplerFilter filter, SamplerMode mode, Vec4 borderColor, float mipBias, float minMIP, float maxMIP, unsigned int maxAnisotropy);
		GP_DLL ~GfxSampler();

		inline ID3D11SamplerState* GetSampler() const { return m_Sampler; }

	private:
		inline unsigned int GetDefaultMaxAnisotropy(SamplerFilter filter) const { return filter == SamplerFilter::Anisotropic ? 16 : 0; }
		inline float GetDefaultMaxMip(SamplerFilter filter) const { return filter == SamplerFilter::Trilinear || filter == SamplerFilter::Anisotropic ? MAX_MIP_VALUE : 0.0f; }

	private:
		ID3D11SamplerState* m_Sampler;
	};
}