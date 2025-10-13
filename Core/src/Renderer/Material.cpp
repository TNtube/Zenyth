#include "pch.hpp"
#include "Renderer/Material.hpp"

#include "Application.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/Renderer.hpp"

namespace Zenyth
{
	Material::Material(const MaterialDesc& matDesc, DescriptorHeap& resourceHeap)
		: m_constantBuffer(resourceHeap), m_description(matDesc)
	{
		m_pipeline = std::make_unique<Pipeline>();

		const std::wstring wName(matDesc.name.begin(), matDesc.name.end());
		const std::wstring wVsPath(matDesc.vsPath.begin(), matDesc.vsPath.end());
		const std::wstring wPsPath(matDesc.psPath.begin(), matDesc.psPath.end());

		m_pipeline->Create(wName, wVsPath, wPsPath, true);

		m_diffuseMap = Texture::LoadTextureFromFile(matDesc.diffuseMap.c_str(), resourceHeap);
		m_normalMap = Texture::LoadTextureFromFile(matDesc.normalMap.c_str(), resourceHeap, false);
		m_specularMap = Texture::LoadTextureFromFile(matDesc.specularMap.c_str(), resourceHeap);
	}

	void Material::Submit(ID3D12GraphicsCommandList* commandList) const
	{
		commandList->SetPipelineState(m_pipeline->Get());
		commandList->SetGraphicsRootSignature(m_pipeline->GetRootSignature());

		const auto albedoIdx   = m_pipeline->GetRootParameterIndex("AlbedoMap");
		const auto normalIdx   = m_pipeline->GetRootParameterIndex("NormalMap");
		const auto specularIdx = m_pipeline->GetRootParameterIndex("SpecularMap");

		if (albedoIdx.has_value() && m_diffuseMap)
			commandList->SetGraphicsRootDescriptorTable(albedoIdx.value(), m_diffuseMap->GetSRV().GPU());
		if (normalIdx.has_value() && m_normalMap)
			commandList->SetGraphicsRootDescriptorTable(normalIdx.value(), m_normalMap->GetSRV().GPU());
		if (specularIdx.has_value() && m_specularMap)
			commandList->SetGraphicsRootDescriptorTable(specularIdx.value(), m_specularMap->GetSRV().GPU());
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

		auto& renderer = Application::Get().GetRenderer();
		return m_materialInstances.emplace_back(new Material(matDesc, renderer.GetResourceHeap()));
	}
}
