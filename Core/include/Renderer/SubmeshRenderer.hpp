#pragma once
#include "Buffers.hpp"
#include "Material.hpp"
#include "Data/Submesh.hpp"


class SubmeshRenderer
{
public:
	explicit SubmeshRenderer(const Submesh& submesh);

	void Submit(CommandBatch& commandBatch) const;
private:
	std::shared_ptr<Material> m_material;
	VertexBuffer m_vertexBuffer;
	IndexBuffer m_indexBuffer;
};