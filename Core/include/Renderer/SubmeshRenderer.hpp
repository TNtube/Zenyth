#pragma once
#include "Buffers.hpp"
#include "Material.hpp"
#include "Data/Submesh.hpp"

namespace Zenyth
{
	class SubmeshRenderer
	{
	public:
		explicit SubmeshRenderer(const Submesh& submesh, DescriptorHeap& resourceHeap);

		void Submit(CommandBatch& commandBatch) const;
	private:
		Material m_material;
		VertexBuffer m_vertexBuffer;
		IndexBuffer m_indexBuffer;
	};
}
