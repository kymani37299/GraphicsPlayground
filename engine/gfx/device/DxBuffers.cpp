#include "gfx/device/DxBuffers.h"

#include "gfx/device/DxCommon.h"
#include "gfx/device/DxDevice.h"

///////////////////////////////////////////
/// DX Functions                     /////
/////////////////////////////////////////

ID3D11Buffer* CreateConstantBuffer(DxDevice* device, unsigned int bufferSize)
{
    ID3D11Buffer* buffer = nullptr;

    D3D11_BUFFER_DESC bufferDesc = {};
    // ByteWidth must be a multiple of 16, per the docs
    bufferDesc.ByteWidth = bufferSize + 0xf & 0xfffffff0;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    DX_CALL(device->GetDevice()->CreateBuffer(&bufferDesc, nullptr, &buffer));

    return buffer;
}

ID3D11Buffer* CreateStructuredBuffer(DxDevice* device, unsigned int elementSize, unsigned int numElements)
{
    ID3D11Buffer* buffer = nullptr;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = elementSize * numElements;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = elementSize;

    DX_CALL(device->GetDevice()->CreateBuffer(&bufferDesc, NULL, &buffer));

    return buffer;
}

ID3D11ShaderResourceView* CreateStructuredBufferView(DxDevice* device, ID3D11Buffer* structuredBuffer, unsigned int numElements)
{
    ID3D11ShaderResourceView* srv;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = numElements;

    DX_CALL(device->GetDevice()->CreateShaderResourceView(structuredBuffer, &srvDesc, &srv));

    return srv;
}

void ReleaseBuffer(ID3D11Buffer* buffer)
{
    buffer->Release();
}

void ReleaseSRV(ID3D11ShaderResourceView* srv)
{
    srv->Release();
}

void UploadToBuffer(DxDevice* device, ID3D11Buffer* buffer, const void* data, unsigned int numBytes, unsigned int offset)
{
    ID3D11DeviceContext1* deviceContext = device->GetDeviceContext();

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    DX_CALL(deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource));
    byte* bufferPtr = (byte*)mappedSubresource.pData;
    memcpy(bufferPtr + offset, data, numBytes);
    deviceContext->Unmap(buffer, 0);
}

///////////////////////////////////////////
/// Vertex buffer                    /////
/////////////////////////////////////////

DxVertexBuffer::DxVertexBuffer(DxDevice* device, const VertexBufferData& data)
{
    m_Stride = data.stride;
    m_NumVerts = data.numBytes / data.stride;
    m_Offset = 0;

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = data.numBytes;
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexSubresourceData = { data.pData };

    DX_CALL(device->GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &m_Buffer));
}

DxVertexBuffer::~DxVertexBuffer()
{
    if(m_BufferOwner)
        m_Buffer->Release();
}

///////////////////////////////////////////
/// Index buffer                     /////
/////////////////////////////////////////

DxIndexBuffer::DxIndexBuffer(DxDevice* device, unsigned int* pIndices, unsigned int numIndices):
    m_NumIndices(numIndices)
{
    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.ByteWidth = sizeof(unsigned int) * numIndices;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexSubresourceData = { pIndices };

    DX_CALL(device->GetDevice()->CreateBuffer(&indexBufferDesc, &vertexSubresourceData, &m_Buffer));
}

DxIndexBuffer::~DxIndexBuffer()
{
    if(m_BufferOwner)
        m_Buffer->Release();
}

///////////////////////////////////////////
/// Buffer allocator                 /////
/////////////////////////////////////////

DxBufferAllocator* DxBufferAllocator::s_Instance = nullptr;

DxBufferAllocator::DxBufferAllocator(DxDevice* device)
{
#ifdef USE_UNIFIED_BUFFERS
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = MAX_BUFFER_SIZE;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    DX_CALL(device->GetDevice()->CreateBuffer(&bufferDesc, NULL, &m_VertexBuffer));

    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    DX_CALL(device->GetDevice()->CreateBuffer(&bufferDesc, NULL, &m_IndexBuffer));
#endif
}

DxBufferAllocator::~DxBufferAllocator()
{
#ifdef USE_UNIFIED_BUFFERS
    m_VertexBuffer->Release();
    m_IndexBuffer->Release();
#endif
}

DxVertexBuffer* DxBufferAllocator::AllocateVertexBuffer(DxDevice* device, const VertexBufferData& data)
{
#ifdef USE_UNIFIED_BUFFERS
    ASSERT(m_VertexFilledSize + data.numBytes <= MAX_BUFFER_SIZE, "[DxBufferAllocator] Out of vertex buffer memory");
    UploadToBuffer(device, m_VertexBuffer, data.pData, data.numBytes, m_VertexFilledSize);

    DxVertexBuffer* vb = new DxVertexBuffer();
    vb->m_Buffer = m_VertexBuffer;
    vb->m_Stride = data.stride;
    vb->m_NumVerts = data.numBytes / data.stride;
    vb->m_Offset = m_VertexFilledSize;
    vb->m_BufferOwner = false;
    
    m_VertexFilledSize += data.numBytes;

    return vb;
#else
    return new DxVertexBuffer(device, data);
#endif
}

DxIndexBuffer* DxBufferAllocator::AllocateIndexBuffer(DxDevice* device, unsigned int* pIndices, unsigned int numIndices)
{
#ifdef USE_UNIFIED_BUFFERS
    unsigned int byteSize = sizeof(unsigned int) * numIndices;
    ASSERT(m_IndexFilledSize + byteSize <= MAX_BUFFER_SIZE, "[DxBufferAllocator] Out of index buffer memory");

    UploadToBuffer(device, m_IndexBuffer, pIndices, byteSize, m_IndexFilledSize);
    
    DxIndexBuffer* ib = new DxIndexBuffer();
    ib->m_Buffer = m_IndexBuffer;
    ib->m_NumIndices = numIndices;
    ib->m_Offset = m_IndexFilledSize;
    ib->m_BufferOwner = false;

    m_IndexFilledSize += byteSize;

    return ib;
#else
    return new DxIndexBuffer(device, pIndices, numIndices);
#endif
}