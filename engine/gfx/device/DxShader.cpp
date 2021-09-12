#include "gfx/device/DxShader.h"

#include <d3dcompiler.h>

#include <vector>

#include "core/Core.h"
#include "util/StringUtil.h"
#include "gfx/device/DxCommon.h"
#include "gfx/device/DxDevice.h"

DXGI_FORMAT ToDXGIFormat(ShaderInputFormat format)
{
    switch (format)
    {
    case ShaderInputFormat::Float:
        return DXGI_FORMAT_R32_FLOAT;
    case ShaderInputFormat::Float2:
        return DXGI_FORMAT_R32G32_FLOAT;
    case ShaderInputFormat::Float3:
        return DXGI_FORMAT_R32G32B32_FLOAT;
    case ShaderInputFormat::Float4:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    default:
        NOT_IMPLEMENTED;
    }
    return DXGI_FORMAT_UNKNOWN;
}

inline bool CompileShader(ID3D11Device1* device, const ShaderDesc& desc, ID3D11VertexShader*& vs, ID3D11PixelShader*& ps, ID3D11InputLayout*& il)
{
    std::wstring wsPath = StringUtil::ToWideString(desc.path);

    // Create Vertex Shader
    ID3DBlob* vsBlob;
    {
        ID3DBlob* shaderCompileErrorsBlob;
        HRESULT hResult = D3DCompileFromFile(wsPath.c_str(), nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, &shaderCompileErrorsBlob);
        if (FAILED(hResult))
        {
            const char* errorString = NULL;
            if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                errorString = "Could not compile shader; file not found";
            else if (shaderCompileErrorsBlob) {
                errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
                shaderCompileErrorsBlob->Release();
            }
            MessageBoxA(0, errorString, "VS Shader Compiler Error", MB_ICONERROR | MB_OK);
            return false;
        }

        hResult = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs);
        ASSERT(SUCCEEDED(hResult), "VS Shader create fail");
    }

    // Create Pixel Shader
    if(!desc.skipPS)
    {
        ID3DBlob* psBlob;
        ID3DBlob* shaderCompileErrorsBlob;
        HRESULT hResult = D3DCompileFromFile(wsPath.c_str(), nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorsBlob);
        if (FAILED(hResult))
        {
            const char* errorString = NULL;
            if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                errorString = "Could not compile shader; file not found";
            else if (shaderCompileErrorsBlob) {
                errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
                shaderCompileErrorsBlob->Release();
            }
            MessageBoxA(0, errorString, "PS Shader Compiler Error", MB_ICONERROR | MB_OK);
            return false;
        }

        hResult = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps);
        ASSERT(SUCCEEDED(hResult), "PS Shader create fail");
        psBlob->Release();
    }

    // Create Input Layout
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements(desc.inputs.size());
    size_t inputIndex = 0;
    for (ShaderInput input : desc.inputs)
    {
        D3D11_INPUT_ELEMENT_DESC& elementDesc = inputElements[inputIndex];
        elementDesc.SemanticName = input.semanticName;
        elementDesc.SemanticIndex = 0;
        elementDesc.Format = ToDXGIFormat(input.format);
        elementDesc.InputSlot = 0;
        elementDesc.AlignedByteOffset = inputIndex == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
        elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        elementDesc.InstanceDataStepRate = 0;

        inputIndex++;
    }

    HRESULT hResult = device->CreateInputLayout(inputElements.data(), inputElements.size(), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &il);
    vsBlob->Release();
    return SUCCEEDED(hResult);
}

DxShader::DxShader(DxDevice* device, const ShaderDesc& desc)
{
#ifdef DEBUG
    m_Desc = desc;
#endif

    bool success = CompileShader(device->GetDevice(), desc, m_VertexShader, m_PixelShader, m_InputLayout);
    ASSERT(success, "Shader compilation failed!");
    m_Initialized = true;
}

DxShader::~DxShader()
{
    m_VertexShader->Release();
    if(m_PixelShader)
        m_PixelShader->Release();
    m_InputLayout->Release();
}

#ifdef DEBUG
void DxShader::Reload(DxDevice* device)
{
    ID3D11VertexShader* vs;
    ID3D11PixelShader* ps = nullptr;
    ID3D11InputLayout* il;

    if (CompileShader(device->GetDevice(), m_Desc, vs, ps, il))
    {
        m_VertexShader->Release();
        if(m_PixelShader)
            m_PixelShader->Release();
        m_InputLayout->Release();

        m_VertexShader = vs;
        m_PixelShader = ps;
        m_InputLayout = il;
    }
}
#endif