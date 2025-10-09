#pragma once
#include "Core.hpp"

#include <dxcapi.h>
#include <d3d12shader.h>

namespace Zenyth
{
	enum class ShaderType
	{
		Vertex,
		Compute,
		Pixel
	};

	class Pipeline
	{
	public:
		Pipeline() = default;

		DELETE_COPY_CTOR(Pipeline)
		DEFAULT_MOVE_CTOR(Pipeline)

		~Pipeline() = default;

		void Create(const std::wstring &name, const std::wstring &vertexPath, const std::wstring &pixelPath, bool depthBoundsTestSupported);
		void Destroy();

		[[nodiscard]] std::optional<uint32_t> GetRootParameterIndex(const std::string &name) const
		{
			const auto index = m_rootParameterIndices.find(name);
			if (index != m_rootParameterIndices.end())
				return index->second;
			return {};
		}

		[[nodiscard]] ID3D12PipelineState* Get() const { return m_pipelineState.Get(); }
		[[nodiscard]] ID3D12RootSignature* GetRootSignature() const { return m_rootSignature.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

		void InitializeDXC();

		Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring &shaderPath, ShaderType shaderType, const std::wstring &smTarget);

		Microsoft::WRL::ComPtr<IDxcCompiler3> m_compiler;
		Microsoft::WRL::ComPtr<IDxcUtils> m_utils;
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> m_includeHandler;

		std::vector<std::string> m_inputElementSemanticNames{};
		std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElementDescs{};
		std::unordered_map<std::string, uint32_t> m_rootParameterIndices;

		std::vector<CD3DX12_DESCRIPTOR_RANGE1> m_descriptorRanges;
		std::vector<D3D12_ROOT_PARAMETER1> m_rootParameters;
		std::vector<D3D12_STATIC_SAMPLER_DESC> m_staticSamplers;
	};
}
