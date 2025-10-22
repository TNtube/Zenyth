#pragma once
#include "Buffers.hpp"
#include "Pipeline.hpp"
#include "Texture.hpp"
#include "Data/MaterialDesc.hpp"

namespace Zenyth
{
	struct MaterialData
	{
		Vector3 Ambient;
		float _pad0;
		Vector3 Diffuse;
		float _pad1;
		Vector3 Specular;
		float SpecularStrength;
	};


	class Material
	{
	public:
		void Submit(ID3D12GraphicsCommandList* commandList) const;

		[[nodiscard]] const Pipeline& GetPipeline() const { return *m_pipeline; };

		bool operator==(const Material& other) const { return m_description == other.m_description; }
		bool operator==(const MaterialDesc& other) const { return m_description == other; }

	private:
		explicit Material(const MaterialDesc& matDesc);

		friend class MaterialManager;
		std::unique_ptr<Pipeline> m_pipeline;

		ConstantBuffer m_constantBuffer;

		std::unique_ptr<Texture> m_diffuseMap;
		std::unique_ptr<Texture> m_specularMap;
		std::unique_ptr<Texture> m_normalMap;

		MaterialDesc m_description;
	};

	class MaterialManager
	{
	public:
		MaterialManager() = default;
		std::shared_ptr<Material> GetMaterial(const MaterialDesc& matDesc);

	private:
		std::vector<std::shared_ptr<Material>> m_materialInstances {};
	};
}
