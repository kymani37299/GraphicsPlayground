#pragma once

#include "gfx/GfxCommon.h"
#include "gfx/GfxResource.h"

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

	class TextureResource2D : public GfxResourceHandle<ID3D11Texture2D>
	{
	public:
		TextureResource2D(ID3D11Texture2D* resource, unsigned int width, unsigned int height, TextureFormat format, unsigned int numMips, unsigned int arraySize, unsigned int numSamples, unsigned int creationFlags) :
			TextureResource2D(width, height, format, numMips, arraySize, numSamples, creationFlags)
		{
			m_Handle = resource;
		}

		TextureResource2D(unsigned int width, unsigned int height, TextureFormat format, unsigned int numMips, unsigned int arraySize, unsigned int numSamples, unsigned int creationFlags) :
			GfxResourceHandle(creationFlags),
			m_Width(width),
			m_Height(height),
			m_Format(format),
			m_NumMips(numMips),
			m_ArraySize(arraySize),
			m_NumSamples(numSamples)
		{
			ASSERT(m_ArraySize != 0, "[TextureResource2D] ArraySize cannot be 0!");
			ASSERT(m_NumSamples != 0, "[TextureResource2D] NumSamples cannot be 0!");
			ASSERT(!(m_CreationFlags & RCF_Cubemap) || arraySize == 6, "[TextureResource2D] ArraySize must be 6 when BindCubemap flag is enabled.");
		}

		void Initialize(GfxContext* context);

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }
		inline TextureFormat GetFormat() const { return m_Format; }
		inline unsigned int GetNumMips() const { return m_NumMips; }
		inline unsigned int GetArraySize() const { return m_ArraySize; }
		inline unsigned int GetNumSamples() const { return m_NumSamples; }

		inline unsigned int GetRowPitch() const { return m_RowPitch; }
		inline unsigned int GetSlicePitch() const { return m_SlicePitch; }

	private:
		~TextureResource2D();

	private:
		unsigned int m_Width;
		unsigned int m_Height;
		TextureFormat m_Format;
		unsigned int m_NumMips;
		unsigned int m_ArraySize;
		unsigned int m_NumSamples;

		unsigned int m_RowPitch;
		unsigned int m_SlicePitch;
	};

	class TextureResource3D : public GfxResourceHandle<ID3D11Texture3D>
	{
	public:
		TextureResource3D(ID3D11Texture3D* resource, unsigned int width, unsigned int height, unsigned int depth, TextureFormat format, unsigned int numMips, unsigned int creationFlags) :
			TextureResource3D(width, height, depth, format, numMips, creationFlags)
		{
			m_Handle = resource;
		}

		TextureResource3D(unsigned int width, unsigned int height, unsigned int depth, TextureFormat format, unsigned int numMips, unsigned int creationFlags) :
			GfxResourceHandle(creationFlags),
			m_Width(width),
			m_Height(height),
			m_Depth(depth),
			m_Format(format),
			m_NumMips(numMips)
		{
			ASSERT(numMips == 1 || m_CreationFlags & RCF_GenerateMips, "[TextureResource3D] Creating textrure resource with numMips > 1 but flag for Mip generation is off.");
		}

		void Initialize(GfxContext* context);

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }
		inline unsigned int GetDepth() const { return m_Height; }
		inline TextureFormat GetFormat() const { return m_Format; }
		inline unsigned int GetNumMips() const { return m_NumMips; }

	private:
		~TextureResource3D();

	private:
		unsigned int m_Width;
		unsigned int m_Height;
		unsigned int m_Depth;
		TextureFormat m_Format;
		unsigned int m_NumMips;

		unsigned int m_RowPitch;
		unsigned int m_SlicePitch;
	};

	class GfxBaseTexture2D : public GfxResource<TextureResource2D>
	{
	protected:
		using GfxResource::GfxResource;

	public:
		GP_DLL ~GfxBaseTexture2D();
	};

	class GfxBaseTexture3D : public GfxResource<TextureResource3D>
	{
	protected:
		using GfxResource::GfxResource;

	public:
		GP_DLL ~GfxBaseTexture3D();
	};

	class GfxTexture2D : public GfxBaseTexture2D
	{
		static constexpr unsigned int DEFAULT_FLAGS = RCF_SRV;
	public:
		GfxTexture2D(const std::string& path, unsigned int numMips = 1):
			GfxBaseTexture2D(ResourceType::Texture2D)
		{
			m_Resource = new TextureResource2D(0, 0, TextureFormat::RGBA8_UNORM, numMips, 1, 1, DEFAULT_FLAGS);
			std::string paths[1] = { path };
			m_Resource->SetInitializationData(1, paths);
		}

		GfxTexture2D(unsigned int width, unsigned int height, unsigned int numMips = 1, unsigned int numSamples = 1) :
			GfxBaseTexture2D(ResourceType::Texture2D)
		{
			m_Resource = new TextureResource2D(width, height, TextureFormat::RGBA8_UNORM, numMips, 1, numSamples, DEFAULT_FLAGS);
		}

		GfxTexture2D(TextureResource2D* resource) :
			GfxBaseTexture2D(ResourceType::Texture2D, resource)
		{
			m_Resource->AddCreationFlags(DEFAULT_FLAGS);
		}

		inline GfxTexture2D(const GfxBaseTexture2D& texture) : GfxTexture2D(texture.GetResource()) { }
	};

	class GfxTextureArray2D : public GfxBaseTexture2D
	{
		static constexpr unsigned int DEFAULT_FLAGS = RCF_SRV;
	public:
		GfxTextureArray2D(unsigned int width, unsigned int height, unsigned int arraySize, unsigned int numMips = 1, unsigned int numSamples = 1) :
			GfxBaseTexture2D(ResourceType::TextureArray2D)
		{
			m_Resource = new TextureResource2D(width, height, TextureFormat::RGBA8_UNORM, numMips, arraySize, numSamples, DEFAULT_FLAGS);
		}

		GfxTextureArray2D(TextureResource2D* resource) :
			GfxBaseTexture2D(ResourceType::TextureArray2D, resource)
		{
			m_Resource->AddCreationFlags(DEFAULT_FLAGS);
		}

		inline GfxTextureArray2D(const GfxBaseTexture2D& texture) : GfxTextureArray2D(texture.GetResource()) {}
	};

	class GfxCubemap : public GfxBaseTexture2D
	{
		static constexpr unsigned int DEFAULT_FLAGS = RCF_Cubemap | RCF_SRV;
	public:
		// The order of textures:  Right, Left, Up, Down, Back, Front
		GfxCubemap(std::string textures[6], unsigned int numMips = 1):
			GfxBaseTexture2D(ResourceType::Cubemap)
		{
			m_Resource = new TextureResource2D(0, 0, TextureFormat::RGBA8_UNORM, numMips, 6, 1, DEFAULT_FLAGS);
			m_Resource->SetInitializationData(6, textures);
		}

		GfxCubemap(TextureResource2D* resource) :
			GfxBaseTexture2D(ResourceType::Cubemap, resource)
		{
			m_Resource->AddCreationFlags(DEFAULT_FLAGS);
		}

		inline GfxCubemap(const GfxBaseTexture2D& texture) : GfxCubemap(texture.GetResource()) {}
	};

	class GfxTexture3D : public GfxBaseTexture3D
	{
		static constexpr unsigned int DEFAULT_FLAGS = RCF_SRV;
	public:
		GfxTexture3D(unsigned int width, unsigned int height, unsigned int depth, unsigned int numMips = 1) :
			GfxBaseTexture3D(ResourceType::Texture3D)
		{
			m_Resource = new TextureResource3D(width, height, depth, TextureFormat::RGBA8_UNORM, numMips, DEFAULT_FLAGS);
		}

		GfxTexture3D(TextureResource3D* resource) :
			GfxBaseTexture3D(ResourceType::Texture3D, resource)
		{
			m_Resource->AddCreationFlags(DEFAULT_FLAGS);
		}

		inline GfxTexture3D(const GfxBaseTexture3D& resource): GfxTexture3D(resource.GetResource()) {}
	};

	class GfxRWTexture3D : public GfxBaseTexture3D
	{
		static constexpr unsigned int DEFAULT_FLAGS = RCF_UAV;
	public:
		GfxRWTexture3D(unsigned int width, unsigned int height, unsigned int depth, unsigned int numMips = 1) :
			GfxBaseTexture3D(ResourceType::Texture3D)
		{
			m_Resource = new TextureResource3D(width, height, depth, TextureFormat::RGBA8_UNORM, numMips, DEFAULT_FLAGS);
		}

		GfxRWTexture3D(TextureResource3D* resource) :
			GfxBaseTexture3D(ResourceType::Texture3D, resource)
		{
			m_Resource->AddCreationFlags(DEFAULT_FLAGS);
		}

		inline GfxRWTexture3D(const GfxBaseTexture3D& resource): GfxRWTexture3D(resource.GetResource()) { }
	};

	class GfxRenderTarget
	{
		DELETE_COPY_CONSTRUCTOR(GfxRenderTarget);

		static constexpr TextureFormat DEFAULT_RT_FORMAT = TextureFormat::RGBA_FLOAT;
		static constexpr unsigned int DEFAULT_RT_FLAGS = RCF_RT | RCF_SRV;

		static constexpr TextureFormat DEFAULT_DS_FORMAT = TextureFormat::R32_TYPELESS;
		static constexpr TextureFormat DEFAULT_DS_STENCIL_FORMAT = TextureFormat::R24G8_TYPELESS;
		static constexpr unsigned int DEFAULT_DS_FLAGS = RCF_DS | RCF_SRV;
	public:
		static GfxRenderTarget* CreateFromSwapChain(IDXGISwapChain1* swapchain);

	private:
		GfxRenderTarget() {}

	public:
		GfxRenderTarget(unsigned int width, unsigned int height, unsigned int numRTs = 1, bool useDepth = false, bool useStencil = false, unsigned int numSamples = 1):
			m_NumRTs(numRTs),
			m_UseStencil(true),
			m_UseDepth(true),
			m_NumSamples(numSamples)
		{
			InitResources(width, height);
		}

		~GfxRenderTarget()
		{
			FreeResources();
		}

		GP_DLL void Initialize(GfxContext* context);
		GP_DLL void InitResources(unsigned int width, unsigned int height);
		GP_DLL void FreeResources();

		inline bool Initialized() const { return m_Initialized; }
		inline unsigned int GetNumRTs() const { return m_NumRTs; }
		inline bool UseDepth() const { return m_UseDepth; }
		inline bool UseStencil() const { return m_UseStencil; }

		void SetRTSize(unsigned int width, unsigned int height)
		{
			FreeResources();
			InitResources(width, height);
		}

		inline unsigned int GetWidth() const 
		{
			if (m_NumRTs > 0)
				return m_Resources[0]->GetWidth();
			else if (m_UseDepth)
				return m_DepthResource->GetWidth();
			
			ASSERT(0, "[GfxRenderTarget] Internal error!");
			return 0;
		}
		inline unsigned int GetHeight() const 
		{ 
			if (m_NumRTs > 0)
				return m_Resources[0]->GetHeight();
			else if (m_UseDepth)
				return m_DepthResource->GetHeight();

			ASSERT(0, "[GfxRenderTarget] Internal error!");
			return 0;
		}

		inline TextureResource2D* GetResource(unsigned int index = 0) const { return m_Resources[index]; }
		inline TextureResource2D* GetDepthResrouce() const { return m_DepthResource; }

		inline ID3D11RenderTargetView** GetRTVs() const { return (ID3D11RenderTargetView**) m_RTVs.data(); }
		inline ID3D11RenderTargetView* GetRTV(unsigned int index) { return m_RTVs[index]; }
		inline ID3D11DepthStencilView* GetDSV() const { return m_DSV; }

	private:
		bool m_Initialized = false;
		unsigned int m_NumRTs = 1;
		bool m_UseDepth = false;
		bool m_UseStencil = false;
		unsigned int m_NumSamples = 1;

		std::vector<TextureResource2D*> m_Resources;
		std::vector<ID3D11RenderTargetView*> m_RTVs;
		ID3D11DepthStencilView* m_DSV = nullptr;
		TextureResource2D* m_DepthResource = nullptr;

	};

	class GfxCubemapRenderTarget
	{
		DELETE_COPY_CONSTRUCTOR(GfxCubemapRenderTarget);

		static constexpr TextureFormat DEFAULT_RT_FORMAT = TextureFormat::RGBA_FLOAT;
		static constexpr unsigned int DEFAULT_RT_FLAGS = RCF_RT | RCF_SRV;
	public:

		GfxCubemapRenderTarget(unsigned int width, unsigned int height)
		{
			m_Resource = new TextureResource2D(width, height, DEFAULT_RT_FORMAT, 1, 6, 1, DEFAULT_RT_FLAGS);
		}

		GP_DLL ~GfxCubemapRenderTarget();

		inline bool Initialized() const { return m_Initialized; }
		GP_DLL void Initialize(GfxContext* context);

		inline unsigned int GetWidth() const { return m_Resource->GetWidth(); }
		inline unsigned int GetHeight() const { return m_Resource->GetHeight(); }

		inline ID3D11RenderTargetView* GetRTV(unsigned int face) const { return m_RTVs[face]; }

	private:
		bool m_Initialized = false;

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