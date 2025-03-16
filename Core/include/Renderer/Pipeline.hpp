#pragma once
#include "Core.hpp"

#include <dxcapi.h>
#include <d3d12shader.h>

namespace Zenyth
{
	class Pipeline
	{
	public:
		Pipeline() = default;

		DELETE_COPY_CTOR(Pipeline)
		DEFAULT_MOVE_CTOR(Pipeline)

		~Pipeline() = default;

		void Create(const std::wstring& name) const
		{
			IDxcCompiler* compiler = nullptr;
			IDxcLibrary* library = nullptr;
			DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
			DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));

			std::wcout << L"Creating pipeline " << name << compiler << std::endl;
		};

	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	};
}
