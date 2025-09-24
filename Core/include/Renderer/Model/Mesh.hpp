#pragma once
#include "Vertex.hpp"
#include "Renderer/Buffers.hpp"

namespace Zenyth
{
	class Mesh
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name);

		void Draw(ID3D12GraphicsCommandList* commandList) const;

	private:
		VertexBuffer m_vertexBuffer;
		IndexBuffer m_indexBuffer;
	};
}
