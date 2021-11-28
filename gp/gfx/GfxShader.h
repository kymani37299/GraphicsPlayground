#pragma once

#include <string>

#include "gfx/GfxCommon.h"

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11HullShader;
struct ID3D11DomainShader;
struct ID3D11GeometryShader;
struct ID3D11ComputeShader;
struct ID3D11InputLayout;

namespace GP
{
	class GfxDevice;

	class GfxShader
	{
		DELETE_COPY_CONSTRUCTOR(GfxShader);
	public:
		GfxShader::GfxShader(const std::string& path, const std::vector<std::string>& defines = {}):
			m_Defines(defines),
			m_Path(path)
		{ }

		GP_DLL ~GfxShader();
		GP_DLL void Reload();

		void Initialize();
		inline bool IsInitialized() const { return m_Initialized; }
		inline ID3D11VertexShader* GetVS() const { return m_VS; }
		inline ID3D11PixelShader* GetPS() const { return m_PS; }
		inline ID3D11HullShader* GetHS() const { return m_HS; }
		inline ID3D11DomainShader* GetDS() const { return m_DS; }
		inline ID3D11GeometryShader* GetGS() const { return m_GS; }
		inline ID3D11ComputeShader* GetCS() const { return m_CS; }
		inline ID3D11InputLayout* GetIL() const { return m_IL; }
		inline ID3D11InputLayout* GetMIL() const { return m_MIL; }

	private:
		bool m_Initialized = false;

		ID3D11VertexShader* m_VS = nullptr;
		ID3D11PixelShader* m_PS = nullptr;
		ID3D11HullShader* m_HS = nullptr;
		ID3D11DomainShader* m_DS = nullptr;
		ID3D11GeometryShader* m_GS = nullptr;
		ID3D11ComputeShader* m_CS = nullptr;

		ID3D11InputLayout* m_IL;
		ID3D11InputLayout* m_MIL;

		std::vector<std::string> m_Defines;
		std::string m_Path;
	};
}
