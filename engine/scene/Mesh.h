#pragma once

#include <string>

namespace GP
{

	namespace ModelLoading
	{
		struct MeshData;
	}

	class GfxVertexBuffer;
	class GfxIndexBuffer;
	class GfxDevice;

	class Mesh
	{
	public:
		Mesh(const ModelLoading::MeshData& data);
		~Mesh();

		inline GfxVertexBuffer* GetVertexBuffer() const { return m_VertexBuffer; }
		inline GfxIndexBuffer* GetIndexBuffer() const { return m_IndexBuffer; }

	private:
		GfxVertexBuffer* m_VertexBuffer;
		GfxIndexBuffer* m_IndexBuffer;
	};

}