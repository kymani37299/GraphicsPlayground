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
    namespace // Util
    {
        // Return true if line has comment and containt input after that comment
        inline bool HasInComment(const std::string& line, const std::string& input)
        {
            // TODO: Case insensitive

            const size_t commentBegin = line.find("//");
            const size_t inputBegin = line.find(input);
            return commentBegin != -1 && inputBegin != -1 && commentBegin < inputBegin;
        }
    }

    namespace // Device State
    {
        enum class BackfaceCullingMode
        {
            OFF,
            CW,
            CCW,

            Default = OFF
        };

        enum class  StencilOp
        {
            Discard,
            Keep,
            Replace,
            
            Default = Keep
        };

        enum class CompareOp
        {
            Always,
            Equals,
            Less,

            // Defaults
            DepthCompareDefault = Less,
            StencilCompareDefault = Always
        };

        enum class Blend
        {
            Zero,
            One,
            SrcColor,
            SrcColorInv,
            SrcAlpha,
            SrcAlphaInv,
            SrcAlphaSat,
            Src1Color,
            Src1ColorInv,
            Src1Alpha,
            Src1AlphaInv,
            DestColor,
            DestColorInv,
            DestAlpha,
            DestAlphaInv,
            BlendFactor,
            BlendFactorInv,

            // Defaults
            SourceColorDefault = SrcAlpha,
            DestColorDefault = SrcAlphaInv,
            SourceAlphaDefault = One,
            DestAlphaDefault = One
        };

        enum class BlendOp
        {
            Add,
            Substract,
            SubstractInv,
            Min,
            Max,

            // Defaults
            Default = Add,
            AlphaDefault = Add,
        };

        inline D3D11_STENCIL_OP sop2desc(StencilOp op)
        {
            switch (op)
            {
            case StencilOp::Discard:
                return D3D11_STENCIL_OP_ZERO;
            case StencilOp::Keep:
                return D3D11_STENCIL_OP_KEEP;
            case StencilOp::Replace:
                return D3D11_STENCIL_OP_REPLACE;
            default:
                NOT_IMPLEMENTED;
            }

            return D3D11_STENCIL_OP_ZERO;
        }

        inline D3D11_COMPARISON_FUNC GetD3D11Comparison(CompareOp op)
        {
            switch (op)
            {
            case CompareOp::Always:
                return D3D11_COMPARISON_ALWAYS;
            case CompareOp::Equals:
                return D3D11_COMPARISON_EQUAL;
            case CompareOp::Less:
                return D3D11_COMPARISON_LESS;
            default:
                NOT_IMPLEMENTED;
            }

            return D3D11_COMPARISON_ALWAYS;
        }

        inline D3D11_DEPTH_STENCILOP_DESC GetD3D11Desc(StencilOp fail, StencilOp depthFail, StencilOp pass, CompareOp compare)
        {
            return { sop2desc(fail) , sop2desc(depthFail), sop2desc(pass), GetD3D11Comparison(compare) };
        }

        D3D11_BLEND_OP GetDXBlendOp(BlendOp blendOp)
        {
            switch (blendOp)
            {
            case BlendOp::Add: return D3D11_BLEND_OP_ADD;
            case BlendOp::Substract: return D3D11_BLEND_OP_SUBTRACT;
            case BlendOp::SubstractInv: return D3D11_BLEND_OP_REV_SUBTRACT;
            case BlendOp::Min: return D3D11_BLEND_OP_MIN;
            case BlendOp::Max: return D3D11_BLEND_OP_MAX;
            default: NOT_IMPLEMENTED;
            }
            return D3D11_BLEND_OP_ADD;
        }

        D3D11_BLEND GetDXBlend(Blend blend)
        {
            switch (blend)
            {
            case Blend::Zero: return D3D11_BLEND_ZERO;
            case Blend::One: return D3D11_BLEND_ONE;
            case Blend::SrcColor: return D3D11_BLEND_SRC_COLOR;
            case Blend::SrcColorInv: return D3D11_BLEND_INV_SRC_COLOR;
            case Blend::SrcAlpha: return D3D11_BLEND_SRC_ALPHA;
            case Blend::SrcAlphaInv: return D3D11_BLEND_INV_SRC_ALPHA;
            case Blend::SrcAlphaSat: return D3D11_BLEND_SRC_ALPHA_SAT;
            case Blend::Src1Color: return D3D11_BLEND_SRC1_COLOR;
            case Blend::Src1ColorInv: return D3D11_BLEND_INV_SRC1_COLOR;
            case Blend::Src1Alpha: return D3D11_BLEND_SRC1_ALPHA;
            case Blend::Src1AlphaInv: return D3D11_BLEND_INV_SRC1_ALPHA;
            case Blend::DestColor: return D3D11_BLEND_DEST_COLOR;
            case Blend::DestColorInv: return D3D11_BLEND_INV_DEST_COLOR;
            case Blend::DestAlpha: return D3D11_BLEND_DEST_ALPHA;
            case Blend::DestAlphaInv: return D3D11_BLEND_INV_DEST_ALPHA;
            case Blend::BlendFactor: return D3D11_BLEND_BLEND_FACTOR;
            case Blend::BlendFactorInv: return D3D11_BLEND_INV_BLEND_FACTOR;
            default: NOT_IMPLEMENTED;
            }
            return D3D11_BLEND_ZERO;
        }

        struct DeviceState
        {
            // Rasterizer state
            BackfaceCullingMode backfaceCullingMode = BackfaceCullingMode::Default;
            bool wireframeEnabled = false;
            bool multisamplingEnabled = false;

            // Depth state
            bool depthTestEnabled = false;
            bool depthWriteEnabled = true;
            CompareOp depthCompareOp = CompareOp::DepthCompareDefault;

            // Stencil state
            bool stencilEnabled = false;
            unsigned int stencilRead = 0xff;
            unsigned int stencilWrite = 0xff;
            StencilOp stencilOp[3] = { StencilOp::Default, StencilOp::Default, StencilOp::Default };
            CompareOp stencilCompareOp = CompareOp::StencilCompareDefault;

            // Blend state
            bool alphaBlendEnabled = false;
            BlendOp blendOp = BlendOp::Default;
            BlendOp blendAlphaOp = BlendOp::AlphaDefault;
            Blend sourceColorBlend = Blend::SourceColorDefault;
            Blend destColorBlend = Blend::DestColorDefault;
            Blend sourceAlphaBlend = Blend::SourceAlphaDefault;
            Blend destAlphaBlend = Blend::DestAlphaDefault;

            // Draw state
            PrimitiveTopology topology = PrimitiveTopology::Default;
        };

        static const std::unordered_map<std::string, BackfaceCullingMode> BackfaceCullingModeMap = {
            {"BACKFACE_OFF", BackfaceCullingMode::OFF},
            {"BACKFACE_CW", BackfaceCullingMode::CW},
            {"BACKFACE_CCW", BackfaceCullingMode::CCW}
        };

        static const std::unordered_map<std::string, CompareOp> CompareOpMap = {
            {"Always", CompareOp::Always},
            {"Equals", CompareOp::Equals},
            {"Less", CompareOp::Less}
        };

        static const std::unordered_map<std::string, StencilOp> StencilOpMap = {
            {"Discard", StencilOp::Discard },
            {"Keep", StencilOp::Keep },
            {"Replace", StencilOp::Replace }
        };

        static const std::unordered_map<std::string, BlendOp> BlendOpMap = {
            { "Add" , BlendOp::Add },
            { "Substract" , BlendOp::Substract },
            { "SubstractInv" , BlendOp::SubstractInv },
            { "Min" , BlendOp::Min },
            { "Max" , BlendOp::Max }
        };

        static const std::unordered_map<std::string, Blend> BlendMap = {
            {"Zero", Blend::Zero },
            {"One", Blend::One },
            {"SrcColor", Blend::SrcColor },
            {"SrcColorInv", Blend::SrcColorInv },
            {"SrcAlpha", Blend::SrcAlpha },
            {"SrcAlphaInv", Blend::SrcAlphaInv },
            {"SrcAlphaSat", Blend::SrcAlphaSat },
            {"Src1Color", Blend::Src1Color },
            {"Src1ColorInv", Blend::Src1ColorInv },
            {"Src1Alpha", Blend::Src1Alpha },
            {"Src1AlphaInv", Blend::Src1AlphaInv },
            {"DestColor", Blend::DestColor },
            {"DestColorInv", Blend::DestColorInv },
            {"DestAlpha", Blend::DestAlpha },
            {"DestAlphaInv", Blend::DestAlphaInv },
            {"BlendFactor", Blend::BlendFactor },
            {"BlendFactorInv", Blend::BlendFactorInv }
        };

        static const std::unordered_map<std::string, PrimitiveTopology> PrimitiveTopologyMap = {
            {"Points",PrimitiveTopology::Points },
            {"Lines",PrimitiveTopology::Lines },
            {"LineStrip",PrimitiveTopology::LineStrip },
            {"Triangles",PrimitiveTopology::Triangles },
            {"TriangleStrip",PrimitiveTopology::TriangleStrip },
            {"LinesAdj",PrimitiveTopology::LinesAdj },
            {"LineStripAdj",PrimitiveTopology::LineStripAdj },
            {"TrianglesAdj",PrimitiveTopology::TrianglesAdj },
            {"TriangleStripAdj",PrimitiveTopology::TriangleStripAdj }
        };
    }

    namespace HeaderCompiler
    {
        

        struct HeaderData
        {
            std::string shaderVersion = "5_0";

            std::string vsEntry = "vs_main";
            std::string psEntry = "ps_main";
            std::string hsEntry = "hs_main";
            std::string dsEntry = "ds_main";
            std::string gsEntry = "gs_main";
            std::string csEntry = "cs_main";

            bool vsEnabled = true;
            bool psEnabled = true;
            bool hsEnabled = false;
            bool dsEnabled = false;
            bool gsEnabled = false;
            bool csEnabled = false;

            DeviceState deviceState;
        };

        template<typename ReturnValue, ReturnValue defaultValue>
        inline ReturnValue GetValueFromMap(const std::string& line, const std::unordered_map<std::string, ReturnValue>& map, const std::string& prefix = "")
        {
            for (const auto& it : map)
            {
                if (HasInComment(line, prefix + it.first)) return it.second;
            }
            return defaultValue;
        }

        inline void SetShaderStages(const std::string& line, HeaderData& header)
        {
            header.vsEnabled = HasInComment(line, "VS");
            header.psEnabled = HasInComment(line, "PS");
            header.dsEnabled = HasInComment(line, "DS");
            header.hsEnabled = HasInComment(line, "HS");
            header.gsEnabled = HasInComment(line, "GS");
            header.csEnabled = HasInComment(line, "CS");
        }

        inline void SetRasterizerState(const std::string& line, HeaderData& header)
        {
            header.deviceState.backfaceCullingMode = GetValueFromMap<BackfaceCullingMode, BackfaceCullingMode::Default>(line, BackfaceCullingModeMap);
            if (HasInComment(line, "WIREFRAME_MODE")) header.deviceState.wireframeEnabled = true;
            if (HasInComment(line, "MULTISAMPLE")) header.deviceState.multisamplingEnabled = true;
        }

        inline void SetDepthState(const std::string& line, HeaderData& header)
        {
            if (HasInComment(line, "ENABLED")) header.deviceState.depthTestEnabled = true;
            if (HasInComment(line, "DEPTH_WRITE_OFF")) header.deviceState.depthWriteEnabled = false;
            header.deviceState.depthCompareOp = GetValueFromMap<CompareOp, CompareOp::DepthCompareDefault>(line, CompareOpMap, "DepthCompare_");
        }

        inline void SetStencilState(const std::string& line, HeaderData& header)
        {
            // TODO
            NOT_IMPLEMENTED;
        }

        inline void SetBlendState(const std::string& line, HeaderData& header)
        {
            if (HasInComment(line, "ENABLED")) header.deviceState.alphaBlendEnabled = true;
            header.deviceState.blendOp = GetValueFromMap<BlendOp, BlendOp::Default>(line, BlendOpMap, "ColorOp_");
            header.deviceState.blendAlphaOp = GetValueFromMap<BlendOp, BlendOp::AlphaDefault>(line, BlendOpMap, "AlphaOp_");
            header.deviceState.sourceColorBlend = GetValueFromMap<Blend, Blend::SourceColorDefault>(line, BlendMap, "SrcColor_");
            header.deviceState.destColorBlend = GetValueFromMap<Blend, Blend::DestColorDefault>(line, BlendMap, "DstColor_");
            header.deviceState.sourceAlphaBlend = GetValueFromMap<Blend, Blend::SourceAlphaDefault>(line, BlendMap, "SrcAlpha_");
            header.deviceState.destAlphaBlend = GetValueFromMap<Blend, Blend::DestAlphaDefault>(line, BlendMap, "DstAlpha_");
        }

        inline void SetPrimitiveTopology(const std::string& line, HeaderData& header)
        {
            header.deviceState.topology = GetValueFromMap<PrimitiveTopology, PrimitiveTopology::Default>(line, PrimitiveTopologyMap);
        }

        void CompileHeader(std::vector<std::string>& shaderCode, HeaderData& header)
        {
            for (const std::string& line : shaderCode)
            {
                // If doesn't contain comment skip the line
                if (!StringUtil::Contains(line, "//")) continue;

                if (HasInComment(line, "ShaderStages")) SetShaderStages(line, header);
                else if (HasInComment(line, "RasterizerState")) SetRasterizerState(line, header);
                else if (HasInComment(line, "DepthState")) SetDepthState(line, header);
                else if (HasInComment(line, "StencilState")) SetStencilState(line, header);
                else if (HasInComment(line, "BlendState")) SetBlendState(line, header);
                else if (HasInComment(line, "Topology")) SetPrimitiveTopology(line, header);
            }
        }
    }

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

            ID3D11DepthStencilState* depthStencilState = nullptr;
            ID3D11RasterizerState1* rasterizerState = nullptr;
            ID3D11BlendState1* blendState = nullptr;
            PrimitiveTopology topology = PrimitiveTopology::Default;
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

        static void ReadShaderFile(std::string path, std::string& shaderCode, HeaderCompiler::HeaderData& headerData)
        {
            static const std::string commonInclude = "gp/gfx/GPShaderCommon.h";

            shaderCode = "";

            std::string rootPath = PathUtil::GetPathWitoutFile(path);
            std::vector<std::string> shaderContent;
            std::vector<std::string> tmp;

            bool readSuccess = false;

            readSuccess = ReadFile(path, shaderContent);
            ASSERT(readSuccess, "Failed to load shader: " + path);

            // TODO: Apply defines on shader content so we can have multiple variations of header
            HeaderCompiler::CompileHeader(shaderContent, headerData);

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

            bool lastPerInstance = false; // Last input was perInstance
            std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements(desc.InputParameters);
            for (size_t i = 0; i < desc.InputParameters; i++)
            {
                D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
                reflection->GetInputParameterDesc(i, &paramDesc);

                const bool perInstance = std::string(paramDesc.SemanticName).find("I_") == 0 ||
                                         std::string(paramDesc.SemanticName).find("i_") == 0;

                // If we have mixed inputs don't create single slot input layout
                if (i > 0 && !multiInput && perInstance != lastPerInstance) 
                {
                    reflection->Release();
                    return nullptr;
                }
                
                inputElements[i].SemanticName = paramDesc.SemanticName;
                inputElements[i].SemanticIndex = paramDesc.SemanticIndex;
                inputElements[i].Format = ToDXGIFormat(paramDesc);
                inputElements[i].InputSlot = multiInput ? i : 0;
                inputElements[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
                inputElements[i].InputSlotClass = perInstance ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
                inputElements[i].InstanceDataStepRate = perInstance ? 1 : 0;

                lastPerInstance = perInstance;
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
            HeaderCompiler::HeaderData header;
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

            // Compile state

            const DeviceState state = header.deviceState;

            D3D11_RASTERIZER_DESC1 rDesc = {};
            rDesc.FillMode = state.wireframeEnabled ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
            rDesc.CullMode = state.backfaceCullingMode != BackfaceCullingMode::OFF ? D3D11_CULL_BACK : D3D11_CULL_NONE;
            rDesc.FrontCounterClockwise = true;
            rDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
            rDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
            rDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            rDesc.DepthClipEnable = true;
            rDesc.ScissorEnable = false;
            rDesc.MultisampleEnable = state.multisamplingEnabled;
            rDesc.AntialiasedLineEnable = false;
            rDesc.ForcedSampleCount = 0;
            result.success = result.success && SUCCEEDED(device->CreateRasterizerState1(&rDesc, &result.rasterizerState));

            CD3D11_DEPTH_STENCIL_DESC dsDesc;
            dsDesc.DepthEnable = state.depthTestEnabled;
            dsDesc.DepthWriteMask = state.depthTestEnabled && state.depthWriteEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
            dsDesc.DepthFunc = GetD3D11Comparison(state.depthCompareOp);

            const D3D11_DEPTH_STENCILOP_DESC stencilOp = GetD3D11Desc(state.stencilOp[0], state.stencilOp[1], state.stencilOp[2], state.stencilCompareOp);
            dsDesc.StencilEnable = state.stencilEnabled;
            dsDesc.StencilReadMask = state.stencilRead;
            dsDesc.StencilWriteMask = state.stencilRead;
            dsDesc.FrontFace = stencilOp;
            dsDesc.BackFace = stencilOp;
            result.success = result.success && SUCCEEDED(device->CreateDepthStencilState(&dsDesc, &result.depthStencilState));

            // Blend
            D3D11_RENDER_TARGET_BLEND_DESC1 rtbDesc = {};
            rtbDesc.BlendEnable = state.alphaBlendEnabled;
            rtbDesc.BlendOp = GetDXBlendOp(state.blendOp);
            rtbDesc.BlendOpAlpha = GetDXBlendOp(state.blendAlphaOp);
            rtbDesc.SrcBlend = GetDXBlend(state.sourceColorBlend);
            rtbDesc.DestBlend = GetDXBlend(state.destColorBlend);
            rtbDesc.SrcBlendAlpha = GetDXBlend(state.sourceAlphaBlend);
            rtbDesc.DestBlendAlpha = GetDXBlend(state.destAlphaBlend);
            rtbDesc.LogicOpEnable = false;
            rtbDesc.LogicOp = D3D11_LOGIC_OP_NOOP;
            rtbDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

            D3D11_BLEND_DESC1 bDesc = {};
            bDesc.AlphaToCoverageEnable = false;
            bDesc.IndependentBlendEnable = false;
            bDesc.RenderTarget[0] = rtbDesc;

            result.success = result.success && SUCCEEDED(device->CreateBlendState1(&bDesc, &result.blendState));

            result.topology = state.topology;

            return result;
        }
    }

    ///////////////////////////////////////
    //			Shader  		        //
    /////////////////////////////////////

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
        SAFE_RELEASE(m_DepthStencilState);
        SAFE_RELEASE(m_RasterizerState);
        SAFE_RELEASE(m_BlendState);
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

            m_DepthStencilState = compiledShader.depthStencilState;
            m_RasterizerState = compiledShader.rasterizerState;
            m_BlendState = compiledShader.blendState;
            m_Topology = compiledShader.topology;
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
            SAFE_RELEASE(compiledShader.depthStencilState);
            SAFE_RELEASE(compiledShader.rasterizerState);
            SAFE_RELEASE(compiledShader.blendState);
        }
    }

    void GfxShader::Initialize()
    {
        ShaderCompiler::CompiledShader compiledShader = ShaderCompiler::CompileShader(m_Path, m_Defines);
        m_Initialized = compiledShader.success;
        ASSERT(m_Initialized, "[GfxShader] Shader comilation failed for shader: " + m_Path);
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

            m_DepthStencilState = compiledShader.depthStencilState;
            m_RasterizerState = compiledShader.rasterizerState;
            m_BlendState = compiledShader.blendState;
            m_Topology = compiledShader.topology;
        }
    }
}