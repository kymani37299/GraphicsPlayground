#pragma once

#include "core/Core.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;

class DxBufferAllocator;
class DxDevice;

struct VertexBufferData
{
	void* pData;
	unsigned int numBytes;
	unsigned int stride;
};

class DxVertexBuffer
{
	friend class DxBufferAllocator;

public:
	DxVertexBuffer(DxDevice* device, const VertexBufferData& data);
	~DxVertexBuffer();

	inline unsigned int GetStride() const { return m_Stride; }
	inline unsigned int GetOffset() const { return m_Offset; }
	inline unsigned int GetNumVerts() const { return m_NumVerts; }
	inline ID3D11Buffer* GetBuffer() const { return m_Buffer; }

private:
	DxVertexBuffer() {}

private:
	unsigned int m_Stride = 0;
	unsigned int m_Offset = 0;
	unsigned int m_NumVerts = 0;

	bool m_BufferOwner = true;
	ID3D11Buffer* m_Buffer = nullptr;
};

class DxIndexBuffer
{
	friend class DxBufferAllocator;

public:
	DxIndexBuffer(DxDevice* device, unsigned int* pIndices, unsigned int numIndices);
	~DxIndexBuffer();

	inline ID3D11Buffer* GetBuffer() const { return m_Buffer; }
	inline unsigned int GetOffset() const { return m_Offset; }
	inline unsigned int GetNumIndices() const { return m_NumIndices; }

private:
	DxIndexBuffer() {}

private:
	unsigned int m_Offset = 0;
	unsigned int m_NumIndices = 0;

	bool m_BufferOwner = true;
	ID3D11Buffer* m_Buffer = nullptr;
};

template<typename T>
class DxConstantBuffer
{
public:
	DxConstantBuffer(DxDevice* device);
	~DxConstantBuffer();

	void Upload(const T& data);

	inline ID3D11Buffer* GetBuffer() const { return m_Buffer; }
private:
	unsigned int m_Size;

	DxDevice* m_Device;

	ID3D11Buffer* m_Buffer;
};

template<typename T>
class DxStructuredBuffer
{
public:
	DxStructuredBuffer(DxDevice* device, unsigned int numElements);
	~DxStructuredBuffer();

	void Upload(const T& data, unsigned int index);

	inline ID3D11ShaderResourceView* GetSRV() const { return m_Srv; }

private:
	unsigned int m_NumElements;
	unsigned int m_ElementSize;

	DxDevice* m_Device;

	ID3D11Buffer* m_Buffer;
	ID3D11ShaderResourceView* m_Srv;
};

class DxBufferAllocator
{
	static constexpr unsigned int MAX_BUFFER_SIZE = 300 * 1024 * 1024; // 300MB
public:
	static DxBufferAllocator* s_Instance;
	static void Init(DxDevice* device) { ASSERT(!s_Instance, "[DxBufferAllocator::Init] s_Instance != null"); s_Instance = new DxBufferAllocator(device); }
	static void Deinit() { SAFE_DELETE(s_Instance); }
	static DxBufferAllocator* Get() { return s_Instance; }

private:
	DxBufferAllocator(DxDevice* device);
	~DxBufferAllocator();

public:
	DxVertexBuffer* AllocateVertexBuffer(DxDevice* device, const VertexBufferData& data);
	DxIndexBuffer* AllocateIndexBuffer(DxDevice* device, unsigned int* pIndices, unsigned int numIndices);

private:
	unsigned int m_VertexFilledSize = 0;
	ID3D11Buffer* m_VertexBuffer;

	unsigned int m_IndexFilledSize = 0;
	ID3D11Buffer* m_IndexBuffer;
};

#include "DxBuffers.hpp"