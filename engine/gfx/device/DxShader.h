#pragma once

#include <string>
#include <vector>

struct ID3D11InputLayout;
struct ID3D11VertexShader;
struct ID3D11PixelShader;

class DxDevice;

enum class ShaderInputFormat
{
	Float,
	Float2,
	Float3,
	Float4
};

struct ShaderInput
{
	ShaderInputFormat format;
	const char* semanticName;
};

struct ShaderDesc
{
	std::string path;
	std::vector<ShaderInput> inputs;
	bool skipPS = false;
};

class DxShader
{
public:
	DxShader(DxDevice* device, const ShaderDesc& desc);
	~DxShader();

	inline bool IsInitialized() const { return m_Initialized; }

	inline ID3D11VertexShader* GetVertexShader() const { return m_VertexShader; }
	inline ID3D11PixelShader* GetPixelShader() const { return m_PixelShader; }
	inline ID3D11InputLayout* GetInputLayout() const { return m_InputLayout; }

private:
	bool m_Initialized = false;

	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_InputLayout;

#ifdef DEBUG
public:
	void Reload(DxDevice* device);

private:
	ShaderDesc m_Desc;
#endif
};