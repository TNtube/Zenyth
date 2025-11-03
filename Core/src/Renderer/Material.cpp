#include "pch.hpp"
#include "Renderer/Material.hpp"

#include "Application.hpp"
#include "Renderer/CommandBatch.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/Renderer.hpp"

Material::Material(const MaterialDesc& matDesc)
	: m_description(matDesc)
{
	m_pipeline = std::make_unique<Pipeline>();

	const std::wstring wName(matDesc.name.begin(), matDesc.name.end());
	const std::wstring wVsPath(matDesc.vsPath.begin(), matDesc.vsPath.end());
	const std::wstring wPsPath(matDesc.psPath.begin(), matDesc.psPath.end());

	m_pipeline->Create(wName, wVsPath, wPsPath, true);

	auto diff = TextureManager::GetTexture(matDesc.diffuseMap);
	if (!diff) diff = TextureManager::GetDefault(DefaultTexture::Magenta);
	auto norm = TextureManager::GetTexture(matDesc.normalMap, false);
	if (!norm) norm = TextureManager::GetDefault(DefaultTexture::Normal);
	auto spec = TextureManager::GetTexture(matDesc.specularMap);
	if (!spec) spec = TextureManager::GetDefault(DefaultTexture::Black);

	m_diffuseMap = diff->GetSRV();
	m_normalMap = norm->GetSRV();
	m_specularMap = spec->GetSRV();
}

void Material::Submit(CommandBatch& commandBatch) const
{
	commandBatch.SetPipeline(*m_pipeline);
	commandBatch.SetRootParameter(3, m_diffuseMap);
	commandBatch.SetRootParameter(4, m_normalMap);
	commandBatch.SetRootParameter(5, m_specularMap);
}

std::shared_ptr<Material> MaterialManager::GetMaterial(const MaterialDesc& matDesc)
{
	const auto mat = std::ranges::find_if(
		m_materialInstances,
		[&matDesc](const auto& curr){
			return *curr == matDesc;
		});

	if (mat != m_materialInstances.end())
		return *mat;

	return m_materialInstances.emplace_back(new Material(matDesc));
}