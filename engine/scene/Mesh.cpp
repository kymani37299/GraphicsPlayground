#include "Mesh.h"

#include "core/GfxCore.h"
#include "util/ModelLoading.h"

namespace GP
{
	Mesh::Mesh(const ModelLoading::MeshData& data)
	{
		VertexBufferData vertexData = {};
		vertexData.pData = data.pVertices;
		vertexData.stride = ModelLoading::MeshVertex::GetStride();
		vertexData.numBytes = sizeof(ModelLoading::MeshVertex) * data.numVertices;

		m_VertexBuffer = new GfxVertexBuffer(vertexData);
		m_IndexBuffer = new GfxIndexBuffer(data.pIndices, data.numIndices);
	}

	Mesh::~Mesh()
	{
		delete m_IndexBuffer;
		delete m_VertexBuffer;
	}

}