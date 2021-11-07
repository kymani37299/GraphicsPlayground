#pragma once

#include <string>

#include "gfx/GfxCommon.h"

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11ComputeShader;
struct ID3D11InputLayout;

struct ID3D10Blob;
typedef ID3D10Blob ID3DBlob;

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
		ID3D11InputLayout* mil = nullptr;
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
		inline void SetConfiguration(const std::vector<std::string>& configuration) { m_Configuration = configuration; }

		CompiledShader CompileShader(const std::string& path);

	private:
		GfxDevice* m_Device;

		std::string m_PSEntry = "";
		std::string m_VSEntry = "";
		std::string m_CSEntry = "";
		std::vector<std::string> m_Configuration = {};

		ID3DBlob* m_VSBlob = nullptr;
		ID3DBlob* m_PSBlob = nullptr;
		ID3DBlob* m_CSBlob = nullptr;
	};

	class GfxShader
	{
		const std::string DEFAULT_VS_ENTRY = "vs_main";
		const std::string DEFAULT_PS_ENTRY = "ps_main";

		DELETE_COPY_CONSTRUCTOR(GfxShader);
	public:
		GP_DLL GfxShader(const std::string& path, const std::vector<std::string>& configuration = {}, bool skipPS = false);
		GP_DLL GfxShader(const std::string& path, const std::string& vsEntry, const std::string& psEntry, const std::vector<std::string>& configuration = {}, bool skipPS = false);
		GP_DLL ~GfxShader();

		GP_DLL void Reload();
		inline bool IsInitialized() const { return m_Initialized; }

		inline ID3D11VertexShader* GetVertexShader() const { return m_VertexShader; }
		inline ID3D11PixelShader* GetPixelShader() const { return m_PixelShader; }
		inline ID3D11InputLayout* GetInputLayout() const { return m_InputLayout; }
		inline ID3D11InputLayout* GetMultiInputLayout() const { return m_MultiInputLayout; }

	private:
		bool CompileShader(const std::string& path, const std::string& vsEntry, const std::string psEntry, const std::vector<std::string>& configuration);

	private:
		bool m_Initialized = false;

		ID3D11VertexShader* m_VertexShader;
		ID3D11PixelShader* m_PixelShader = nullptr;
		ID3D11InputLayout* m_InputLayout;
		ID3D11InputLayout* m_MultiInputLayout;

#ifdef DEBUG
		std::string m_Path;
		std::vector<std::string> m_Configuration;
		std::string m_VSEntry = DEFAULT_VS_ENTRY;
		std::string m_PSEntry = DEFAULT_PS_ENTRY;
#endif // DEBUG
	};

	class GfxComputeShader
	{
		const std::string DEFAULT_ENTRY = "cs_main";

		DELETE_COPY_CONSTRUCTOR(GfxComputeShader);
	public:
		GP_DLL GfxComputeShader(const std::string& path, const std::vector<std::string>& configuration = {});
		GP_DLL GfxComputeShader(const std::string& path, const std::string& entryPoint, const std::vector<std::string>& configuration = {});
		GP_DLL ~GfxComputeShader();

		GP_DLL void Reload();
		inline bool IsInitialized() const { return m_Initialized; }

		inline ID3D11ComputeShader* GetShader() const { return m_Shader; }

	private:
		bool CompileShader(const std::string& path, const std::string& entry, const std::vector<std::string>& configuration);

	private:
		bool m_Initialized = false;
		ID3D11ComputeShader* m_Shader;

#ifdef DEBUG
		std::string m_Path;
		std::vector<std::string> m_Configuration;
		std::string m_Entry = DEFAULT_ENTRY;
#endif // DEBUG
	};
}
