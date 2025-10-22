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

		void Create(const std::wstring& name, const std::wstring& vertexPath, const std::wstring& pixelPath, bool depthBoundsTestSupported);
		void Create(const std::wstring& name, const std::wstring& computePath);
		void Destroy();

		[[nodiscard]] ID3D12PipelineState* Get() const { return m_pipelineState.Get(); }
		[[nodiscard]] ID3D12RootSignature* GetRootSignature() const { return m_rootSignature.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

		void InitializeDXC();

		Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& shaderPath, ShaderType shaderType, const std::wstring& smTarget);

		Microsoft::WRL::ComPtr<IDxcCompiler3> m_compiler;
		Microsoft::WRL::ComPtr<IDxcUtils> m_utils;
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> m_includeHandler;

		std::vector<std::string> m_inputElementSemanticNames{};
		std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElementDescs{};
	};
}
