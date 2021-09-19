#pragma once

#include <string>

#include "Common.h"

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11ComputeShader;
struct ID3D11InputLayout;

#include <d3dcommon.h>
//struct ID3DBlob;

namespace GP
{
	class GfxDevice;

	struct CompiledShader
	{
		bool valid = false;
		ID3D11VertexShader* vs = nullptr;
		ID3D11PixelShader* ps = nullptr;
		ID3D11ComputeShader* cs = nullptr;
		ID3D11InputLayout* il = nullptr;
	};

	class ShaderFactory
	{
		const std::string VS_TARGET = "vs_5_0";
		const std::string PS_TARGET = "ps_5_0";
		const std::string CS_TARGET = "cs_5_0";

	public:
		ShaderFactory(GfxDevice* device) : m_Device(device) {}

		inline void SetPSEntry(const std::string& entryPoint) { m_PSEntry = entryPoint; }
		inline void SetVSEntry(const std::string& entryPoint) { m_VSEntry = entryPoint; }
		inline void SetCSEntry(const std::string& entryPoint) { m_CSEntry = entryPoint; }

		CompiledShader CompileShader(const std::string& path);

	private:
		GfxDevice* m_Device;

		std::string m_PSEntry = "";
		std::string m_VSEntry = "";
		std::string m_CSEntry = "";

		ID3DBlob* m_VSBlob = nullptr;
		ID3DBlob* m_PSBlob = nullptr;
		ID3DBlob* m_CSBlob = nullptr;
	};
}
