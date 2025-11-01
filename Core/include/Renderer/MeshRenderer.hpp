#pragma once
#include "SubmeshRenderer.hpp"
#include "Data/Mesh.hpp"

class MeshRenderer
{
public:
	explicit MeshRenderer(const Mesh& mesh);

	void Submit(CommandBatch& commandBatch) const;

private:
	std::vector<std::shared_ptr<SubmeshRenderer>> m_submeshRenderers;
};