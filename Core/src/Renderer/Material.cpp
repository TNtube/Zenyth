#include "pch.hpp"
#include "Renderer/Material.hpp"

#include "Application.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/Renderer.hpp"

namespace Zenyth
{
	Material::Material(const MaterialDesc& matDesc)
		: m_description(matDesc)
	{
		m_pipeline = std::make_unique<Pipeline>();

		const std::wstring wName(matDesc.name.begin(), matDesc.name.end());
		const std::wstring wVsPath(matDesc.vsPath.begin(), matDesc.vsPath.end());
		const std::wstring wPsPath(matDesc.psPath.begin(), matDesc.psPath.end());

		m_pipeline->Create(wName, wVsPath, wPsPath, true);

		m_diffuseMap = Texture::LoadTextureFromFile(matDesc.diffuseMap.c_str());
		m_normalMap = Texture::LoadTextureFromFile(matDesc.normalMap.c_str(), false);
		m_specularMap = Texture::LoadTextureFromFile(matDesc.specularMap.c_str());
	}

	void Material::Submit(ID3D12GraphicsCommandList* commandList) const
	{
		commandList->SetPipelineState(m_pipeline->Get());
		commandList->SetGraphicsRootSignature(m_pipeline->GetRootSignature());

		if (m_diffuseMap)
			commandList->SetGraphicsRootDescriptorTable(3, m_diffuseMap->GetSRV().GPU());
		if (m_normalMap)
			commandList->SetGraphicsRootDescriptorTable(4, m_normalMap->GetSRV().GPU());
		if (m_specularMap)
			commandList->SetGraphicsRootDescriptorTable(5, m_specularMap->GetSRV().GPU());
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
}
