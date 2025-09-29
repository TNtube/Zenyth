#pragma once
#include "Buffers.hpp"
#include "Data/Submesh.hpp"

namespace Zenyth
{
	class SubmeshRenderer
	{
	public:
		explicit SubmeshRenderer(const Submesh& submesh);

		void Draw(ID3D12GraphicsCommandList* commandList) const;
	private:
		VertexBuffer m_vertexBuffer;
		IndexBuffer m_indexBuffer;
	};
}
