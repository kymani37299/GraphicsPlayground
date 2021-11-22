#pragma once

#include <d3d11_1.h>

#include "gfx/GfxResource.h"

namespace GP
{
	namespace
	{
		inline D3D11_USAGE GetUsageFlags(unsigned int creationFlags)
		{
			if (creationFlags & RCF_CPUWrite)			return D3D11_USAGE_DYNAMIC;
			else if (creationFlags & RCF_CPURead)		return D3D11_USAGE_STAGING;
			else if (creationFlags & RCF_UAV)			return D3D11_USAGE_DEFAULT;
			else if (creationFlags & RCF_CopyDest)		return D3D11_USAGE_DEFAULT;
			else if (creationFlags & RCF_RT)			return D3D11_USAGE_DEFAULT;
			else if (creationFlags & RCF_DS)			return D3D11_USAGE_DEFAULT;
			else										return D3D11_USAGE_IMMUTABLE;
		}

		inline unsigned int GetBindFlags(unsigned int creationFlags)
		{
			unsigned int flags = 0;
			if (creationFlags & RCF_SRV)				flags |= D3D11_BIND_SHADER_RESOURCE;
			if (creationFlags & RCF_UAV)				flags |= D3D11_BIND_UNORDERED_ACCESS;
			if (creationFlags & RCF_RT)					flags |= D3D11_BIND_RENDER_TARGET;
			if (creationFlags & RCF_DS)					flags |= D3D11_BIND_DEPTH_STENCIL;
			if (creationFlags & RCF_VB)					flags |= D3D11_BIND_VERTEX_BUFFER;
			if (creationFlags & RCF_IB)					flags |= D3D11_BIND_INDEX_BUFFER;
			if (creationFlags & RCF_CB)					flags |= D3D11_BIND_CONSTANT_BUFFER;
			return flags;
		}

		inline unsigned int GetAccessFlags(unsigned int creationFlags)
		{
			unsigned int flags = 0;
			if (creationFlags & RCF_CPURead)	flags |= D3D11_CPU_ACCESS_READ;
			if (creationFlags & RCF_CPUWrite)	flags |= D3D11_CPU_ACCESS_WRITE;
			return flags;
		}

		inline unsigned int GetMiscFlags(unsigned int creationFlags)
		{
			unsigned int flags = 0;
			if (creationFlags & RCF_GenerateMips)		flags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
			if (creationFlags & RCF_Cubemap)			flags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
			if (creationFlags & RCF_StructuredBuffer)	flags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			return flags;
		}

		inline D3D11_BUFFER_DESC GetBufferDesc(unsigned int byteSize, unsigned int creationFlags, unsigned int structByteStride)
		{
			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.ByteWidth = byteSize;
			bufferDesc.Usage = GetUsageFlags(creationFlags);
			bufferDesc.BindFlags = GetBindFlags(creationFlags);
			bufferDesc.CPUAccessFlags = GetAccessFlags(creationFlags);
			bufferDesc.MiscFlags = GetMiscFlags(creationFlags);
			bufferDesc.StructureByteStride = creationFlags & RCF_StructuredBuffer ? structByteStride : 0;
			return bufferDesc;
		}

		inline D3D11_TEXTURE2D_DESC FillTexture2DDescription(unsigned int width, unsigned int height, unsigned int numMips, unsigned int arraySize, DXGI_FORMAT format, unsigned int creationFlags)
		{
			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width = width;
			textureDesc.Height = height;
			textureDesc.MipLevels = numMips;
			textureDesc.ArraySize = arraySize;
			textureDesc.Format = format;
			textureDesc.SampleDesc.Count = 1; // TODO: Support MSAA
			textureDesc.Usage = GetUsageFlags(creationFlags);
			textureDesc.BindFlags = GetBindFlags(creationFlags);
			textureDesc.MiscFlags = GetMiscFlags(creationFlags);
			textureDesc.CPUAccessFlags = GetAccessFlags(creationFlags);
			return textureDesc;
		}

		inline D3D11_TEXTURE3D_DESC Fill3DTextureDescription(unsigned int width, unsigned int height, unsigned int depth, unsigned int numMips, DXGI_FORMAT format, unsigned int creationFlags)
		{
			D3D11_TEXTURE3D_DESC textureDesc = {};
			textureDesc.Width = width;
			textureDesc.Height = height;;
			textureDesc.Depth = depth;
			textureDesc.MipLevels = numMips;
			textureDesc.Format = format;
			textureDesc.Usage = GetUsageFlags(creationFlags);
			textureDesc.BindFlags = GetBindFlags(creationFlags);
			textureDesc.CPUAccessFlags = GetAccessFlags(creationFlags);
			textureDesc.MiscFlags = GetMiscFlags(creationFlags);
			return textureDesc;
		}
	}
}