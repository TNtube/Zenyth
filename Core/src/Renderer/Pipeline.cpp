#include "pch.hpp"

#include "Renderer/Pipeline.hpp"

#include <ranges>

#include "Application.hpp"
#include "CommonStates.h"
#include "Renderer/Renderer.hpp"


using namespace Microsoft::WRL;

void Pipeline::Create(const std::wstring& name, const std::wstring& vertexPath, const std::wstring& pixelPath, bool depthBoundsTestSupported)
{
	Destroy();
	auto& renderer = Application::Get().GetRenderer();

	m_shader.AddStage(ShaderStage::Vertex, vertexPath);
	m_shader.AddStage(ShaderStage::Pixel, pixelPath);

	auto psoDesc = m_shader.GetGraphicPipelineStateDesc();
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
	ThrowIfFailed(renderer.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

	SUCCEEDED(m_pipelineState->SetName(name.c_str()));

}

void Pipeline::Create(const std::wstring& name, const std::wstring& computePath)
{
	Destroy();
	const auto& renderer = Application::Get().GetRenderer();

	m_shader.AddStage(ShaderStage::Compute, computePath);
	const auto computeDesc = m_shader.GetComputePipelineStateDesc();

	ThrowIfFailed( renderer.GetDevice()->CreateComputePipelineState( &computeDesc, IID_PPV_ARGS( &m_pipelineState ) ) );

	m_pipelineState->SetName(name.c_str());
}

void Pipeline::Destroy()
{
	m_pipelineState.Reset();
	m_shader.Destroy();
}