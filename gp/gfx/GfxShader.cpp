#include "GfxShader.h"

#include <d3d11_1.h>
#include <d3dcompiler.h>

#include <string>
#include <fstream>
#include <vector>
#include <set>

#include "gfx/GfxDevice.h"
#include "util/StringUtil.h"
#include "util/PathUtil.h"

namespace GP
{
    namespace ShaderCompiler
    {
        struct CompiledShader
        {
            bool success = false;

            ID3D11VertexShader* vs = nullptr;
            ID3D11PixelShader* ps = nullptr;
            ID3D11HullShader* hs = nullptr;
            ID3D11DomainShader* ds = nullptr;
            ID3D11GeometryShader* gs = nullptr;
            ID3D11ComputeShader* cs = nullptr;

            ID3D11InputLayout* il = nullptr;
            ID3D11InputLayout* mil = nullptr;
        };

        struct HeaderData
        {
            std::string shaderVersion = "5_0";

            std::string vsEntry = "vs_main";
            std::string psEntry = "ps_main";
            std::string hsEntry = "hs_main";
            std::string dsEntry = "ds_main";
            std::string gsEntry = "gs_main";
            std::string csEntry = "cs_main";

            bool vsEnabled = false;
            bool psEnabled = false;
            bool hsEnabled = false;
            bool dsEnabled = false;
            bool gsEnabled = false;
            bool csEnabled = false;
        };

        DXGI_FORMAT ToDXGIFormat(D3D11_SIGNATURE_PARAMETER_DESC paramDesc)
        {
            if (paramDesc.Mask == 1)
            {
                switch (paramDesc.ComponentType)
                {
                case D3D_REGISTER_COMPONENT_UINT32:
                    return DXGI_FORMAT_R32_UINT;
                case D3D_REGISTER_COMPONENT_SINT32:
                    return DXGI_FORMAT_R32_SINT;
                case D3D_REGISTER_COMPONENT_FLOAT32:
                    return DXGI_FORMAT_R32_FLOAT;
                }
            }
            else if (paramDesc.Mask <= 3)
            {
                switch (paramDesc.ComponentType)
                {
                case D3D_REGISTER_COMPONENT_UINT32:
                    return DXGI_FORMAT_R32G32_UINT;
                case D3D_REGISTER_COMPONENT_SINT32:
                    return DXGI_FORMAT_R32G32_SINT;
                case D3D_REGISTER_COMPONENT_FLOAT32:
                    return DXGI_FORMAT_R32G32_FLOAT;
                }
            }
            else if (paramDesc.Mask <= 7)
            {
                switch (paramDesc.ComponentType)
                {
                case D3D_REGISTER_COMPONENT_UINT32:
                    return DXGI_FORMAT_R32G32B32_UINT;
                case D3D_REGISTER_COMPONENT_SINT32:
                    return DXGI_FORMAT_R32G32B32_SINT;
                case D3D_REGISTER_COMPONENT_FLOAT32:
                    return DXGI_FORMAT_R32G32B32_FLOAT;
                }
            }
            else if (paramDesc.Mask <= 15)
            {
                switch (paramDesc.ComponentType)
                {
                case D3D_REGISTER_COMPONENT_UINT32:
                    return DXGI_FORMAT_R32G32B32A32_UINT;
                case D3D_REGISTER_COMPONENT_SINT32:
                    return DXGI_FORMAT_R32G32B32A32_SINT;
                case D3D_REGISTER_COMPONENT_FLOAT32:
                    return DXGI_FORMAT_R32G32B32A32_FLOAT;
                }
            }

            NOT_IMPLEMENTED;
            return DXGI_FORMAT_UNKNOWN;
        }

        static bool ReadFile(const std::string& path, std::vector<std::string>& content)
        {
            content.clear();

            std::ifstream fileStream(path, std::ios::in);

            if (!fileStream.is_open()) {
                return false;
            }

            std::string line = "";
            while (!fileStream.eof()) {
                std::getline(fileStream, line);
                content.push_back(line);
            }

            fileStream.close();
            return true;
        }

        static void ReadHeader(const std::vector<std::string>& shaderContent, HeaderData& headerData)
        {
            const std::string& firstLine = shaderContent[0];
            if (StringUtil::Contains(firstLine, "//") && StringUtil::Contains(firstLine, "ShaderStages"))
            {
                headerData.vsEnabled = StringUtil::Contains(firstLine, "VS");
                headerData.psEnabled = StringUtil::Contains(firstLine, "PS");
                headerData.dsEnabled = StringUtil::Contains(firstLine, "DS");
                headerData.hsEnabled = StringUtil::Contains(firstLine, "HS");
                headerData.gsEnabled = StringUtil::Contains(firstLine, "GS");
                headerData.csEnabled = StringUtil::Contains(firstLine, "CS");
            }
            else
            {
                headerData.vsEnabled = true;
                headerData.psEnabled = true;
            }
        }

        static void ReadShaderFile(std::string path, std::string& shaderCode, HeaderData& headerData)
        {
            static const std::string commonInclude = "gp/gfx/GPShaderCommon.h";

            shaderCode = "";

            std::string rootPath = PathUtil::GetPathWitoutFile(path);
            std::vector<std::string> shaderContent;
            std::vector<std::string> tmp;

            bool readSuccess = false;

            readSuccess = ReadFile(path, shaderContent);
            ASSERT(readSuccess, "Failed to load shader: " + path);

            ReadHeader(shaderContent, headerData);

            readSuccess = ReadFile(commonInclude, tmp);
            ASSERT(readSuccess, "Failed to include common shader header!");

            shaderContent.insert((shaderContent.begin()), tmp.begin(), tmp.end());

            std::set<std::string> loadedFiles = {};
            for (size_t i = 0; i < shaderContent.size(); i++)
            {
                std::string& line = shaderContent[i];
                if (StringUtil::Contains(line, "#include"))
                {
                    std::string fileName = line;
                    StringUtil::ReplaceAll(fileName, "#include", "");
                    StringUtil::ReplaceAll(fileName, " ", "");
                    StringUtil::ReplaceAll(fileName, "\"", "");

                    if (loadedFiles.count(fileName)) continue;
                    loadedFiles.insert(fileName);

                    readSuccess = ReadFile(rootPath + fileName, tmp);
                    ASSERT(readSuccess, "Failed to include file in shader: " + rootPath + fileName);
                    shaderContent.insert((shaderContent.begin() + (i + 1)), tmp.begin(), tmp.end());
                }
                else
                {
                    shaderCode.append(line + "\n");
                }
            }
        }

        ID3DBlob* ReadBlobFromFile(const std::string& shaderCode, const std::string& entry, const std::string& hlsl_target, D3D_SHADER_MACRO* configuration)
        {
            ID3DBlob *shaderCompileErrorsBlob, *blob;
            HRESULT hResult = D3DCompile(shaderCode.c_str(), shaderCode.size(), nullptr, configuration, nullptr,  entry.c_str(), hlsl_target.c_str(), 0, 0, &blob, &shaderCompileErrorsBlob);
            if (FAILED(hResult))
            {
                const char* errorString = NULL;
                if (shaderCompileErrorsBlob) {
                    errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
                    shaderCompileErrorsBlob->Release();
                }
                MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
                return nullptr;
            }
            return blob;
        }

        ID3D11InputLayout* CreateInputLayout(ID3D11Device1* device, ID3DBlob* vsBlob, bool multiInput)
        {
            ID3D11ShaderReflection* reflection;
            DX_CALL(D3DReflect(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection));
            D3D11_SHADER_DESC desc;
            reflection->GetDesc(&desc);

            std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements(desc.InputParameters);
            for (size_t i = 0; i < desc.InputParameters; i++)
            {
                D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
                reflection->GetInputParameterDesc(i, &paramDesc);

                inputElements[i].SemanticName = paramDesc.SemanticName;
                inputElements[i].SemanticIndex = paramDesc.SemanticIndex;
                inputElements[i].Format = ToDXGIFormat(paramDesc);
                inputElements[i].InputSlot = multiInput ? i : 0;
                inputElements[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
                inputElements[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                inputElements[i].InstanceDataStepRate = 0;
            }

            ID3D11InputLayout* inputLayout;
            DX_CALL(device->CreateInputLayout(inputElements.data(), desc.InputParameters, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout));

            reflection->Release();

            return inputLayout;
        }

        D3D_SHADER_MACRO* CompileConfiguration(const std::vector<std::string>& configuration)
        {
            if (configuration.size() == 0) return nullptr;

            const size_t numConfigs = configuration.size();
            D3D_SHADER_MACRO* compiledConfig = (D3D_SHADER_MACRO*) malloc(sizeof(D3D_SHADER_MACRO) * (numConfigs+1));
            for (size_t i = 0; i < numConfigs; i++)
            {
                compiledConfig[i].Name = configuration[i].c_str();
                compiledConfig[i].Definition = "";
            }
            compiledConfig[numConfigs].Name = NULL;
            compiledConfig[numConfigs].Definition = NULL;

            return compiledConfig;
        }

        CompiledShader CompileShader(const std::string& path, const std::vector<std::string>& defines)
        {
            CompiledShader result;

            std::string shaderCode;
            HeaderData header;
            ReadShaderFile(path, shaderCode, header);

            D3D_SHADER_MACRO* configuration = CompileConfiguration(defines);
            ID3DBlob* vsBlob = header.vsEnabled ? ReadBlobFromFile(shaderCode, header.vsEntry, "vs_" + header.shaderVersion, configuration) : nullptr;
            ID3DBlob* psBlob = header.psEnabled ? ReadBlobFromFile(shaderCode, header.psEntry, "ps_" + header.shaderVersion, configuration) : nullptr;
            ID3DBlob* dsBlob = header.dsEnabled ? ReadBlobFromFile(shaderCode, header.dsEntry, "ds_" + header.shaderVersion, configuration) : nullptr;
            ID3DBlob* hsBlob = header.hsEnabled ? ReadBlobFromFile(shaderCode, header.hsEntry, "hs_" + header.shaderVersion, configuration) : nullptr;
            ID3DBlob* gsBlob = header.gsEnabled ? ReadBlobFromFile(shaderCode, header.gsEntry, "gs_" + header.shaderVersion, configuration) : nullptr;
            ID3DBlob* csBlob = header.csEnabled ? ReadBlobFromFile(shaderCode, header.csEntry, "cs_" + header.shaderVersion, configuration) : nullptr;

            ID3D11Device1* device = g_Device->GetDevice();

            result.success = vsBlob || psBlob || dsBlob || hsBlob || gsBlob || csBlob;
            if (vsBlob) result.success = result.success && SUCCEEDED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &result.vs));
            if (psBlob) result.success = result.success && SUCCEEDED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &result.ps));
            if (dsBlob) result.success = result.success && SUCCEEDED(device->CreateDomainShader(dsBlob->GetBufferPointer(), dsBlob->GetBufferSize(), nullptr, &result.ds));
            if (hsBlob) result.success = result.success && SUCCEEDED(device->CreateHullShader(hsBlob->GetBufferPointer(), hsBlob->GetBufferSize(), nullptr, &result.hs));
            if (gsBlob) result.success = result.success && SUCCEEDED(device->CreateGeometryShader(gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), nullptr, &result.gs));
            if (csBlob) result.success = result.success && SUCCEEDED(device->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(), nullptr, &result.cs));

            if (vsBlob)
            {
                result.il = CreateInputLayout(device, vsBlob, false);
                result.mil = CreateInputLayout(device, vsBlob, true);
            }

            SAFE_RELEASE(vsBlob);
            SAFE_RELEASE(psBlob);
            SAFE_RELEASE(dsBlob);
            SAFE_RELEASE(hsBlob);
            SAFE_RELEASE(gsBlob);
            SAFE_RELEASE(csBlob);

            return result;
        }
    }

    ///////////////////////////////////////
    //			Shader  		        //
    /////////////////////////////////////

    GfxShader::GfxShader(const std::string& path, const std::vector<std::string>& defines):
        m_Defines(defines),
        m_Path(path)
    {
        ShaderCompiler::CompiledShader compiledShader = ShaderCompiler::CompileShader(path, defines);
        m_Initialized = compiledShader.success;
        ASSERT(m_Initialized, "[GfxShader] Shader comilation failed for shader: " + path);
        if (compiledShader.success)
        {
            m_VS = compiledShader.vs;
            m_PS = compiledShader.ps;
            m_HS = compiledShader.hs;
            m_DS = compiledShader.ds;
            m_GS = compiledShader.gs;
            m_CS = compiledShader.cs;
            m_IL = compiledShader.il;
            m_MIL = compiledShader.mil;
        }
    }

    GfxShader::~GfxShader()
    {
        SAFE_RELEASE(m_VS);
        SAFE_RELEASE(m_PS);
        SAFE_RELEASE(m_HS);
        SAFE_RELEASE(m_DS);
        SAFE_RELEASE(m_GS);
        SAFE_RELEASE(m_CS);
        SAFE_RELEASE(m_IL);
        SAFE_RELEASE(m_MIL);
    }

    void GfxShader::Reload()
    {
        ShaderCompiler::CompiledShader compiledShader = ShaderCompiler::CompileShader(m_Path, m_Defines);
        if (compiledShader.success)
        {
            SAFE_RELEASE(m_VS);
            SAFE_RELEASE(m_PS);
            SAFE_RELEASE(m_HS);
            SAFE_RELEASE(m_DS);
            SAFE_RELEASE(m_GS);
            SAFE_RELEASE(m_CS);
            SAFE_RELEASE(m_IL);
            SAFE_RELEASE(m_MIL);

            m_VS = compiledShader.vs;
            m_PS = compiledShader.ps;
            m_HS = compiledShader.hs;
            m_DS = compiledShader.ds;
            m_GS = compiledShader.gs;
            m_CS = compiledShader.cs;
            m_IL = compiledShader.il;
            m_MIL = compiledShader.mil;
        }
        else
        {
            LOG("Reload for shader " + m_Path + " failed!");
            SAFE_RELEASE(compiledShader.vs);
            SAFE_RELEASE(compiledShader.ps);
            SAFE_RELEASE(compiledShader.hs);
            SAFE_RELEASE(compiledShader.ds);
            SAFE_RELEASE(compiledShader.gs);
            SAFE_RELEASE(compiledShader.cs);
            SAFE_RELEASE(compiledShader.il);
            SAFE_RELEASE(compiledShader.mil);
        }
    }
}