#include "pch.hpp"
#include "Renderer/MeshRenderer.hpp"

namespace Zenyth
{
	MeshRenderer::MeshRenderer(const Mesh& mesh)
	{
		m_submeshRenderers.reserve(mesh.m_submeshes.size());

		for (const auto& submesh : mesh.m_submeshes)
			m_submeshRenderers.emplace_back(submesh);
	}

	void MeshRenderer::Submit(CommandBatch& commandBatch) const
	{
		for (const auto& smr : m_submeshRenderers)
		{
			smr.Submit(commandBatch);
		}
	}
}
