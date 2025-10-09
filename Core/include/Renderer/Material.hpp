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
		explicit Material(const MaterialDesc& matDesc, DescriptorHeap& resourceHeap);

		void Submit(ID3D12GraphicsCommandList* commandList) const;

		const Pipeline& GetPipeline() const { return *m_pipeline; };

	private:
		std::unique_ptr<Pipeline> m_pipeline;

		ConstantBuffer m_constantBuffer;

		std::unique_ptr<Texture> m_diffuseMap;
		std::unique_ptr<Texture> m_specularMap;
		std::unique_ptr<Texture> m_normalMap;
	};
}
