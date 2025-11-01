#include "pch.hpp"

#include "Renderer/Shader.hpp"

#include <ranges>

#include "Application.hpp"
#include "Core.hpp"

struct CompileResult
{
	Microsoft::WRL::ComPtr<IDxcBlob> compiled {};
	std::vector<InputElemData> inputElemsData {};
	Microsoft::WRL::ComPtr<IDxcBlob> rootSigBlob{};
};

struct ShaderDataHasher
{
	std::size_t operator()( const ShaderData& k ) const
	{
		auto seed = std::hash<ShaderStage>()(k.stage);
		seed ^=  std::hash<std::wstring>()(k.path) + 0x9e3779b9 + (seed<<6) + (seed>>2);
		return seed;
	}
};

static std::unordered_map<ShaderData, CompileResult, ShaderDataHasher> s_compilationCache {};


using namespace Microsoft::WRL;

DXGI_FORMAT GetDXGIFormat(D3D_REGISTER_COMPONENT_TYPE componentType, BYTE mask);
std::wstring GetShaderFormat(ShaderStage stage);

void Shader::Destroy()
{
	m_shaderStages.clear();
	m_compiledBlobs.clear();
	m_inputElemsData.clear();
	m_rootSignature.Reset();
	m_isCompute = false;
}

void Shader::AddStage(ShaderStage stage, std::filesystem::path path)
{
	if (stage == ShaderStage::Compute)
	{
		if (!m_shaderStages.empty())
			throw std::invalid_argument("Cannot compile compute shader alongside other stages");
		m_isCompute = true;
	}
	else if (m_isCompute)
		throw std::invalid_argument("Cannot compile compute shader alongside other stages");

	m_shaderStages.emplace_back(stage, std::move(path));
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC Shader::GetGraphicPipelineStateDesc()
{
	if (m_isCompute)
		throw std::exception("Cannot generate graphic pipeline state desc from a compute stage");

	if (m_compiledBlobs.empty())
		Compile();

	const auto& vertexBlob = m_compiledBlobs[ShaderStage::Vertex];
	const auto& hullBlob = m_compiledBlobs[ShaderStage::Hull];
	const auto& domainBlob = m_compiledBlobs[ShaderStage::Domain];
	const auto& geometryBlob = m_compiledBlobs[ShaderStage::Geometry];
	const auto& pixelBlob = m_compiledBlobs[ShaderStage::Pixel];

	if (!vertexBlob) {
		throw std::exception("Cannot generate graphic pipeline state desc without vertex stage");
	}

	m_inputElementsDesc.reserve(m_inputElemsData.size());

	for (const auto& [format, index, name]: m_inputElemsData)
	{
		m_inputElementsDesc.emplace_back(D3D12_INPUT_ELEMENT_DESC{
					.SemanticName = name.c_str(),
					.SemanticIndex = index,
					.Format = format,
					.InputSlot = 0u,
					.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
					.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
					.InstanceDataStepRate = 0u,
				});
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = {m_inputElementsDesc.data(), static_cast<uint32_t>(m_inputElementsDesc.size())};
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS.pShaderBytecode = vertexBlob->GetBufferPointer();
	psoDesc.VS.BytecodeLength = vertexBlob->GetBufferSize();

	if (hullBlob)
	{
		psoDesc.HS.pShaderBytecode = hullBlob->GetBufferPointer();
		psoDesc.HS.BytecodeLength = hullBlob->GetBufferSize();
	}

	if (domainBlob)
	{
		psoDesc.DS.pShaderBytecode = domainBlob->GetBufferPointer();
		psoDesc.DS.BytecodeLength = domainBlob->GetBufferSize();
	}

	if (geometryBlob)
	{
		psoDesc.GS.pShaderBytecode = geometryBlob->GetBufferPointer();
		psoDesc.GS.BytecodeLength = geometryBlob->GetBufferSize();
	}

	if (pixelBlob)
	{
		psoDesc.PS.pShaderBytecode = pixelBlob->GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelBlob->GetBufferSize();
	}

	return psoDesc;
}

D3D12_COMPUTE_PIPELINE_STATE_DESC Shader::GetComputePipelineStateDesc()
{
	if (!m_isCompute)
		throw std::exception("Cannot generate compute pipeline state desc from graphic stages");

	if (m_compiledBlobs.empty())
		Compile();

	const auto& computeBlob = m_compiledBlobs[ShaderStage::Compute];
	if (!computeBlob) {
		throw std::exception("Cannot generate compute pipeline state desc without compute stage");
	}

	D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc {};
	computeDesc.pRootSignature = m_rootSignature.Get();
	computeDesc.CS.pShaderBytecode = computeBlob->GetBufferPointer();
	computeDesc.CS.BytecodeLength = computeBlob->GetBufferSize();

	return computeDesc;
}

bool Shader::Compile()
{
	for (const auto& data : m_shaderStages)
	{
		m_compiledBlobs[data.stage] = CompileShader(data);
	}

	return true;
}

IDxcBlob* Shader::CompileShader(const ShaderData& data)
{
	auto& cachedResult = s_compilationCache[data];

	if (cachedResult.compiled)
	{
		if (cachedResult.inputElemsData.size())
			m_inputElemsData = cachedResult.inputElemsData; // explicit copy, the vector should be quite small

		if (!m_rootSignature && cachedResult.rootSigBlob)
			GenerateRootSignature(cachedResult.rootSigBlob);

		return cachedResult.compiled.Get();
	}

	static ComPtr<IDxcUtils> utils = nullptr;
	static ComPtr<IDxcCompiler3> compiler = nullptr;
	static ComPtr<IDxcIncludeHandler> includeHandler = nullptr;

	if (!utils)
		ThrowIfFailed(::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(utils.ReleaseAndGetAddressOf())));
	if (!compiler)
		ThrowIfFailed(::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(compiler.ReleaseAndGetAddressOf())));
	if (!includeHandler)
		ThrowIfFailed(utils->CreateDefaultIncludeHandler(includeHandler.ReleaseAndGetAddressOf()));

	auto pos = data.path.native().find_last_of(L"\\/");
	auto shaderDir = data.path.native().substr(0, pos);
	auto format = GetShaderFormat(data.stage);

	std::vector compilationArguments
	{
		L"-E", L"main",
		L"-T", format.c_str(),
		L"-I", shaderDir.c_str(),
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
	ThrowIfFailed(utils->LoadFile(data.path.c_str(), nullptr, shaderBlob.GetAddressOf()));

	const DxcBuffer sourceBuffer
	{
		.Ptr = shaderBlob->GetBufferPointer(),
		.Size = shaderBlob->GetBufferSize(),
		.Encoding = 0
	};

	ComPtr<IDxcResult> compileResult;
	auto hr = compiler->Compile(&sourceBuffer,
		compilationArguments.data(), static_cast<uint32_t>(compilationArguments.size()),
		includeHandler.Get(), IID_PPV_ARGS(compileResult.GetAddressOf()));

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

	compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&cachedResult.compiled), nullptr);

	// input layout description
	if (data.stage == ShaderStage::Vertex)
	{
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
		utils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflection));
		D3D12_SHADER_DESC shaderDesc{};
		shaderReflection->GetDesc(&shaderDesc);
		cachedResult.inputElemsData.reserve(shaderDesc.InputParameters);

		for (const uint32_t parameterIndex : std::views::iota(0u, shaderDesc.InputParameters))
		{
			D3D12_SIGNATURE_PARAMETER_DESC signatureParameterDesc{};
			shaderReflection->GetInputParameterDesc(parameterIndex, &signatureParameterDesc);

			cachedResult.inputElemsData.emplace_back(
				GetDXGIFormat(signatureParameterDesc.ComponentType, signatureParameterDesc.Mask),
				signatureParameterDesc.SemanticIndex,
				signatureParameterDesc.SemanticName
			);
		}

		m_inputElemsData = cachedResult.inputElemsData;
	}

	// if we have a root signature, we can early exit
	if (m_rootSignature)
		return cachedResult.compiled.Get();

	// Get the root signature (if embedded)
	ComPtr<IDxcBlob> rootSigBlob;
	ComPtr<IDxcBlobUtf16> rootSigName;
	hr = compileResult->GetOutput(DXC_OUT_ROOT_SIGNATURE,
								   IID_PPV_ARGS(&rootSigBlob),
								   &rootSigName);
	if (SUCCEEDED(hr)) {
		cachedResult.rootSigBlob = rootSigBlob;
		GenerateRootSignature(rootSigBlob);
	}
	else {
		// failed, we crash
		ThrowIfFailed(hr, "No root signature found. Please add an explicit root signature in your shader.");
	}

	return cachedResult.compiled.Get();
}

void Shader::GenerateRootSignature(const ComPtr<IDxcBlob>& rootSigBlob)
{
	const auto& renderer = Application::Get().GetRenderer();
	// Shader has explicit root signature - use it!
	renderer.GetDevice()->CreateRootSignature(0,
							   rootSigBlob->GetBufferPointer(),
							   rootSigBlob->GetBufferSize(),
							   IID_PPV_ARGS(&m_rootSignature));

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
static std::wstring GetShaderFormat(const ShaderStage stage)
{
	switch (stage) {
		case ShaderStage::Vertex:
			return L"vs_6_0";
			case ShaderStage::Hull:
				return L"hs_6_0";
			case ShaderStage::Domain:
				return L"ds_6_0";
			case ShaderStage::Geometry:
				return L"gs_6_0";
			case ShaderStage::Pixel:
				return L"ps_6_0";
			case ShaderStage::Compute:
				return L"cs_6_0";
		}

		throw std::invalid_argument("Invalid shader stage");
	}
