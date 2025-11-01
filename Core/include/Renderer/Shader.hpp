#pragma once

#include <dxcapi.h>

struct InputElemData
{
	DXGI_FORMAT format;
	uint32_t index;
	std::string name;
};

enum class ShaderStage
{
	Vertex,
	Hull,
	Domain,
	Geometry,
	Pixel,
	Compute
};

struct ShaderData
{
	ShaderStage stage;
	std::filesystem::path path;

	bool operator==(const ShaderData&) const = default;
};

/*
 * Encapsulate shader compilation and reflection
 */
class Shader
{
public:
	Shader() = default;

	void Destroy();

	void AddStage(ShaderStage stage, std::filesystem::path path);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetGraphicPipelineStateDesc();
	D3D12_COMPUTE_PIPELINE_STATE_DESC GetComputePipelineStateDesc();

	[[nodiscard]] ID3D12RootSignature* GetRootSignature() const { return m_rootSignature.Get(); }

	bool operator==(const Shader& other) const { return m_shaderStages == other.m_shaderStages; };

private:
	bool Compile();
	IDxcBlob* CompileShader(const ShaderData& data);

	void GenerateRootSignature(const Microsoft::WRL::ComPtr<IDxcBlob>& rootSigBlob);

	std::vector<ShaderData> m_shaderStages;

	std::unordered_map<ShaderStage, IDxcBlob*> m_compiledBlobs;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

	// we need to store the names aside for
	// std::vector<std::string> m_inputElementSemanticNames{};
	// std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElementDescs{};
	std::vector<InputElemData> m_inputElemsData {};
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElementsDesc {};

	bool m_isCompute {};
};
