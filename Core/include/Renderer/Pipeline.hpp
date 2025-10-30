#pragma once
#include "Core.hpp"

#include <d3d12shader.h>

#include "Shader.hpp"

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

		const Shader& GetShader() const { return m_shader; }

		[[nodiscard]] ID3D12PipelineState* Get() const { return m_pipelineState.Get(); }
		[[nodiscard]] ID3D12RootSignature* GetRootSignature() const { return m_shader.GetRootSignature(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		Shader m_shader;

	};
}
