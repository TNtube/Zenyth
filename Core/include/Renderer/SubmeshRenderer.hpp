#pragma once
#include "Buffers.hpp"
#include "Material.hpp"
#include "Data/Submesh.hpp"


class SubmeshRenderer
{
public:
	explicit SubmeshRenderer(const Submesh& submesh);

	[[nodiscard]] uint32_t GetMaterialIndex() const { return m_materialIndex; }
	void Submit(const CommandBatch& commandBatch) const;
private:
	uint32_t m_materialIndex;
	VertexBuffer m_vertexBuffer;
	IndexBuffer m_indexBuffer;
};