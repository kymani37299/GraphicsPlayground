#include "ShaderFactory.h"

#include <d3d11_1.h>
#include <d3dcompiler.h>

#include <string>
#include <fstream>
#include <vector>
#include <set>

#include "gfx/GfxCore.h"
#include "util/StringUtil.h"
#include "util/PathUtil.h"

namespace GP
{
    namespace
    {
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

        static void ReadShaderFile(std::string path, std::string& shaderCode)
        {
            static const std::string commonInclude = "engine/gfx/GPShaderCommon.h";

            shaderCode = "";

            std::string rootPath = PathUtil::GetPathWitoutFile(path);
            std::vector<std::string> shaderContent;
            std::vector<std::string> tmp;

            bool readSuccess = false;

            readSuccess = ReadFile(path, shaderContent);
            ASSERT(readSuccess, "Failed to load shader!");

            readSuccess = ReadFile(commonInclude, tmp);
            ASSERT(readSuccess, "Failed to include common shader header!");

            shaderContent.insert((shaderContent.begin()), tmp.begin(), tmp.end());

            std::set<std::string> loadedFiles = {};
            for (size_t i = 0; i < shaderContent.size(); i++)
            {
                std::string& line = shaderContent[i];
                if (line.find("#include") != std::string::npos)
                {
                    std::string fileName = line;
                    StringUtil::ReplaceAll(fileName, "#include", "");
                    StringUtil::ReplaceAll(fileName, " ", "");
                    StringUtil::ReplaceAll(fileName, "\"", "");

                    if (loadedFiles.count(fileName)) continue;
                    loadedFiles.insert(fileName);

                    readSuccess = ReadFile(rootPath + fileName, tmp);
                    ASSERT(readSuccess, "Failed to include file in shader!");
                    shaderContent.insert((shaderContent.begin() + (i + 1)), tmp.begin(), tmp.end());
                }
                else
                {
                    shaderCode.append(line + "\n");
                }
            }
        }

        ID3DBlob* ReadBlobFromFile(const std::string& path, const std::string& entry, const std::string& hlsl_target /* "vs_5_0" */)
        {
            std::string shaderCode;
            ReadShaderFile(path, shaderCode);

            ID3DBlob *shaderCompileErrorsBlob, *blob;
            HRESULT hResult = D3DCompile(shaderCode.c_str(), shaderCode.size(), nullptr, nullptr /* macros */, nullptr,  entry.c_str(), hlsl_target.c_str(), 0, 0, &blob, &shaderCompileErrorsBlob);
            if (FAILED(hResult))
            {
                const char* errorString = NULL;
                if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                    errorString = "Could not compile shader; file not found";
                else if (shaderCompileErrorsBlob) {
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

    }

	CompiledShader ShaderFactory::CompileShader(const std::string& path)
	{
        HRESULT hr;

        m_VSBlob = m_VSEntry.empty() ? nullptr : ReadBlobFromFile(path, m_VSEntry, VS_TARGET);
        m_PSBlob = m_PSEntry.empty() ? nullptr : ReadBlobFromFile(path, m_PSEntry, PS_TARGET);
        m_CSBlob = m_CSEntry.empty() ? nullptr : ReadBlobFromFile(path, m_CSEntry, CS_TARGET);

        ID3D11Device1* device = m_Device->GetDevice();
        CompiledShader compiledShader = {};

        if (m_VSBlob)
        {
            hr = device->CreateVertexShader(m_VSBlob->GetBufferPointer(), m_VSBlob->GetBufferSize(), nullptr, &compiledShader.vs);
            compiledShader.valid = SUCCEEDED(hr);

            if (m_PSBlob)
            {
                hr = device->CreatePixelShader(m_PSBlob->GetBufferPointer(), m_PSBlob->GetBufferSize(), nullptr, &compiledShader.ps);
                compiledShader.valid &= SUCCEEDED(hr);
            }
            compiledShader.il = CreateInputLayout(device, m_VSBlob, false);
            compiledShader.mil = CreateInputLayout(device, m_VSBlob, true);
        }

        if (m_CSBlob)
        {
            hr = device->CreateComputeShader(m_CSBlob->GetBufferPointer(), m_CSBlob->GetBufferSize(), nullptr, &compiledShader.cs);
            compiledShader.valid = SUCCEEDED(hr);
        }

        return compiledShader;
	}
}