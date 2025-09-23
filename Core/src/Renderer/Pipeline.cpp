#include "pch.hpp"

#include "Renderer/Pipeline.hpp"

#include <ranges>

#include "Renderer/Renderer.hpp"

namespace Zenyth
{
	DXGI_FORMAT GetDXGIFormat(D3D_REGISTER_COMPONENT_TYPE componentType, BYTE mask);
	using namespace Microsoft::WRL;

	void Pipeline::Create(const std::wstring& name, const std::wstring& vertexPath, const std::wstring& pixelPath, bool depthBoundsTestSupported)
	{
		Destroy();

		if (m_utils == nullptr || m_compiler == nullptr || m_includeHandler == nullptr)
			InitializeDXC();

		// allocate arbitrary large size to prevent memory invalidation after shader compilation.
		// TODO: encapsulate this into a class that hold shader specific data and use a utility
		// TODO: function to compute root parameters from all compiled shader to prevent this.
		m_descriptorRanges.reserve(256);

		auto vertexBlob = CompileShader(vertexPath, ShaderType::Vertex, L"vs_6_0");
		auto pixelBlob = CompileShader(pixelPath, ShaderType::Pixel, L"ps_6_0");

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(m_rootParameters.size(), m_rootParameters.data(),
			m_staticSamplers.size(), m_staticSamplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(Renderer::pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		if (FAILED(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error)))
		{
			if (error)
			{
				std::string output = static_cast<char *>(error->GetBufferPointer());
				std::cout << output << std::endl;
			}
			ThrowIfFailed(E_FAIL);
		}
		ThrowIfFailed(Renderer::pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = {m_inputElementDescs.data(), static_cast<uint32_t>(m_inputElementDescs.size())};
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS.pShaderBytecode = vertexBlob->GetBufferPointer();
		psoDesc.VS.BytecodeLength = vertexBlob->GetBufferSize();
		psoDesc.PS.pShaderBytecode = pixelBlob->GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelBlob->GetBufferSize();
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

		if (depthBoundsTestSupported)
		{
			CD3DX12_DEPTH_STENCIL_DESC1 depthStencilDesc(D3D12_DEFAULT);

			depthStencilDesc.DepthBoundsTestEnable = depthBoundsTestSupported;
			depthStencilDesc.DepthEnable = TRUE;
			depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;

			psoDesc.DepthStencilState = depthStencilDesc;
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			psoDesc.NumRenderTargets = 2;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
		} else
		{
			psoDesc.DepthStencilState.DepthEnable = FALSE;
			psoDesc.DepthStencilState.StencilEnable = FALSE;
		}
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(Renderer::pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

	}

	void Pipeline::Destroy()
	{
		m_pipelineState.Reset();
		m_rootSignature.Reset();

		m_inputElementSemanticNames.clear();
		m_inputElementDescs.clear();
		m_rootParameterIndices.clear();
		m_descriptorRanges.clear();
		m_rootParameters.clear();
		m_staticSamplers.clear();

	}

	void Pipeline::InitializeDXC()
	{
		ThrowIfFailed(::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(m_utils.ReleaseAndGetAddressOf())));
		ThrowIfFailed(::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(m_compiler.ReleaseAndGetAddressOf())));
		ThrowIfFailed(m_utils->CreateDefaultIncludeHandler(m_includeHandler.ReleaseAndGetAddressOf()));
	}


	ComPtr<IDxcBlob> Pipeline::CompileShader(const std::wstring& shaderPath, const ShaderType shaderType, const std::wstring& smTarget)
	{
		std::vector compilationArguments
		{
			L"-E",
			L"main",
			L"-T",
			smTarget.c_str(),
			DXC_ARG_PACK_MATRIX_ROW_MAJOR,
			DXC_ARG_WARNINGS_ARE_ERRORS,
			DXC_ARG_ALL_RESOURCES_BOUND,
		};

#ifndef NDEBUG
		compilationArguments.push_back(DXC_ARG_DEBUG); // Enable debug info
		compilationArguments.push_back(L"-Qembed_debug"); // Embed debug info
		compilationArguments.push_back(L"-Od"); // Disable optimizations
#else
		compilationArguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
#endif

		ComPtr<IDxcBlobEncoding> shaderBlob;
		ThrowIfFailed(m_utils->LoadFile(shaderPath.c_str(), nullptr, shaderBlob.GetAddressOf()));

		const DxcBuffer sourceBuffer
		{
			.Ptr = shaderBlob->GetBufferPointer(),
			.Size = shaderBlob->GetBufferSize(),
			.Encoding = 0
		};

		ComPtr<IDxcResult> compileResult;
		const auto hr = m_compiler->Compile(&sourceBuffer,
			compilationArguments.data(), static_cast<uint32_t>(compilationArguments.size()),
			m_includeHandler.Get(), IID_PPV_ARGS(compileResult.GetAddressOf()));

		ComPtr<IDxcBlobUtf8> errors{};
		ThrowIfFailed(compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
		if (errors && errors->GetStringLength() > 0)
		{
			std::stringstream errorsStream;
			errorsStream << "Shader compilation failed with the following errors:\n";
			errorsStream << std::string_view{ errors->GetStringPointer(), errors->GetStringLength() };

			std::cout << errorsStream.str() << std::endl;
			throw std::runtime_error(errorsStream.str());
		}

		ThrowIfFailed(hr);

		// Get shader reflection data.
		ComPtr<IDxcBlob> reflectionBlob{};
		ThrowIfFailed(compileResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr));

		const DxcBuffer reflectionBuffer
		{
			.Ptr = reflectionBlob->GetBufferPointer(),
			.Size = reflectionBlob->GetBufferSize(),
			.Encoding = 0,
		};

		ComPtr<ID3D12ShaderReflection> shaderReflection{};
		m_utils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflection));
		D3D12_SHADER_DESC shaderDesc{};
		shaderReflection->GetDesc(&shaderDesc);

		// input layout description
		if (shaderType == ShaderType::Vertex)
		{
			m_inputElementSemanticNames.reserve(shaderDesc.InputParameters);
			m_inputElementDescs.reserve(shaderDesc.InputParameters);

			for (const uint32_t parameterIndex : std::views::iota(0u, shaderDesc.InputParameters))
			{
				D3D12_SIGNATURE_PARAMETER_DESC signatureParameterDesc{};
				shaderReflection->GetInputParameterDesc(parameterIndex, &signatureParameterDesc);

				m_inputElementSemanticNames.emplace_back(signatureParameterDesc.SemanticName);
				m_inputElementDescs.emplace_back(D3D12_INPUT_ELEMENT_DESC{
							.SemanticName = m_inputElementSemanticNames.back().c_str(),
							.SemanticIndex = signatureParameterDesc.SemanticIndex,
							.Format = GetDXGIFormat(signatureParameterDesc.ComponentType, signatureParameterDesc.Mask),
							.InputSlot = 0u,
							.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
							.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
							.InstanceDataStepRate = 0u,
						});
			}
		}

		// constant buffers
		for (const uint32_t i : std::views::iota(0u, shaderDesc.BoundResources))
		{
			D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{};
			ThrowIfFailed(shaderReflection->GetResourceBindingDesc(i, &shaderInputBindDesc));

			if (shaderInputBindDesc.Type == D3D_SIT_CBUFFER)
			{
				m_rootParameterIndices[shaderInputBindDesc.Name] = static_cast<uint32_t>(m_rootParameters.size());
				CD3DX12_DESCRIPTOR_RANGE1 cbvRange;
				cbvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, shaderInputBindDesc.BindPoint, shaderInputBindDesc.Space, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

				m_descriptorRanges.push_back(cbvRange);

				CD3DX12_ROOT_PARAMETER1 rootParameter;
				rootParameter.InitAsDescriptorTable(1, &m_descriptorRanges.back(), D3D12_SHADER_VISIBILITY_VERTEX);

				m_rootParameters.push_back(rootParameter);
			}

			if (shaderInputBindDesc.Type == D3D_SIT_TEXTURE || shaderInputBindDesc.Type == D3D_SIT_STRUCTURED)
			{
				// For now, each individual texture belongs in its own descriptor table. This can cause the root signature to quickly exceed the 64WORD size limit.
				m_rootParameterIndices[shaderInputBindDesc.Name] = static_cast<uint32_t>(m_rootParameters.size());
				const CD3DX12_DESCRIPTOR_RANGE1 srvRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
										1u,
										shaderInputBindDesc.BindPoint,
										shaderInputBindDesc.Space,
										D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

				m_descriptorRanges.push_back(srvRange);

				const D3D12_ROOT_PARAMETER1 rootParameter
				{
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
					.DescriptorTable =
					{
						.NumDescriptorRanges = 1u,
						.pDescriptorRanges = &m_descriptorRanges.back(),
					},
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
				};

				m_rootParameters.push_back(rootParameter);
			}

			if (shaderInputBindDesc.Type == D3D_SIT_SAMPLER)
			{
				const D3D12_STATIC_SAMPLER_DESC staticSampler
				{
					.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT,
					.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
					.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
					.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
					.MipLODBias = 0,
					.MaxAnisotropy = 0,
					.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
					.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
					.MinLOD = 0.0f,
					.MaxLOD = D3D12_FLOAT32_MAX,
					.ShaderRegister = shaderInputBindDesc.BindPoint,
					.RegisterSpace = shaderInputBindDesc.Space,
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
				};

				m_staticSamplers.push_back(staticSampler);
			}
		}

		ComPtr<IDxcBlob> compiledShaderBlob{nullptr};
		compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiledShaderBlob), nullptr);

		return compiledShaderBlob;
	}


	static DXGI_FORMAT GetDXGIFormat(D3D_REGISTER_COMPONENT_TYPE componentType, BYTE mask) {
		// Count the number of components
		int componentCount = 0;
		if (mask & D3D_COMPONENT_MASK_X) componentCount++;
		if (mask & D3D_COMPONENT_MASK_Y) componentCount++;
		if (mask & D3D_COMPONENT_MASK_Z) componentCount++;
		if (mask & D3D_COMPONENT_MASK_W) componentCount++;

		// Determine the format based on the component type and count
		switch (componentType) {
			case D3D_REGISTER_COMPONENT_FLOAT32:
				switch (componentCount) {
					case 1: return DXGI_FORMAT_R32_FLOAT;
					case 2: return DXGI_FORMAT_R32G32_FLOAT;
					case 3: return DXGI_FORMAT_R32G32B32_FLOAT;
					case 4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
					default: return DXGI_FORMAT_UNKNOWN;
				}

			case D3D_REGISTER_COMPONENT_SINT32:
				switch (componentCount) {
					case 1: return DXGI_FORMAT_R32_SINT;
					case 2: return DXGI_FORMAT_R32G32_SINT;
					case 3: return DXGI_FORMAT_R32G32B32_SINT;
					case 4: return DXGI_FORMAT_R32G32B32A32_SINT;
					default: return DXGI_FORMAT_UNKNOWN;
				}

			case D3D_REGISTER_COMPONENT_UINT32:
				switch (componentCount) {
					case 1: return DXGI_FORMAT_R32_UINT;
					case 2: return DXGI_FORMAT_R32G32_UINT;
					case 3: return DXGI_FORMAT_R32G32B32_UINT;
					case 4: return DXGI_FORMAT_R32G32B32A32_UINT;
					default: return DXGI_FORMAT_UNKNOWN;
				}

			case D3D_REGISTER_COMPONENT_FLOAT16:
				switch (componentCount) {
					case 1: return DXGI_FORMAT_R16_FLOAT;
					case 2: return DXGI_FORMAT_R16G16_FLOAT;
					case 4: return DXGI_FORMAT_R16G16B16A16_FLOAT;
					// No 3-component half format
					default: return DXGI_FORMAT_UNKNOWN;
				}

			case D3D_REGISTER_COMPONENT_SINT16:
				switch (componentCount) {
					case 1: return DXGI_FORMAT_R16_SINT;
					case 2: return DXGI_FORMAT_R16G16_SINT;
					case 4: return DXGI_FORMAT_R16G16B16A16_SINT;
					// No 3-component half format
					default: return DXGI_FORMAT_UNKNOWN;
				}

			case D3D_REGISTER_COMPONENT_UINT16:
				switch (componentCount) {
					case 1: return DXGI_FORMAT_R16_UINT;
					case 2: return DXGI_FORMAT_R16G16_UINT;
					case 4: return DXGI_FORMAT_R16G16B16A16_UINT;
					// No 3-component half format
					default: return DXGI_FORMAT_UNKNOWN;
				}
			default: return DXGI_FORMAT_UNKNOWN;
		}

	}
}
