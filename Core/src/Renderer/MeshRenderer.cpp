#include "pch.hpp"
#include "Renderer/MeshRenderer.hpp"

#include "Application.hpp"


MeshRenderer::MeshRenderer(const Mesh& mesh)
{
	auto& renderer = Application::Get().GetRenderer();

	m_submeshRenderers.reserve(mesh.m_submeshes.size());
	m_materials.reserve(mesh.m_materialDesc.size());

	for (const auto& submesh : mesh.m_submeshes)
		m_submeshRenderers.emplace_back(std::make_shared<SubmeshRenderer>(submesh));

	std::ranges::sort(m_submeshRenderers, [](const auto& smra, const auto& smrb) {
		return smra->GetMaterialIndex() < smrb->GetMaterialIndex();
	});

	for (const auto& matDesc : mesh.m_materialDesc)
		m_materials.emplace_back(renderer.GetMaterialManager().GetMaterial(matDesc));
}

void MeshRenderer::Submit(CommandBatch& commandBatch) const
{
	auto lastIdx = -1;
	for (const auto& smr : m_submeshRenderers)
	{
		if (lastIdx != smr->GetMaterialIndex())
		{
			m_materials[smr->GetMaterialIndex()]->Submit(commandBatch);
			lastIdx = smr->GetMaterialIndex();
		}

		smr->Submit(commandBatch);
	}
}