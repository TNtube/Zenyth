#include "pch.hpp"
#include "Renderer/SwapChain.hpp"

#include "Core.hpp"
#include "Win32Application.hpp"

using namespace Microsoft::WRL;

SwapChain::SwapChain(const uint32_t width, const uint32_t height, CommandQueue& graphicQueue)
	: m_graphicQueue(&graphicQueue), m_backBufferTextures(BufferCount)
{
	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)), "Failed to create DXGIFactory");

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = BufferCount;
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	ComPtr<IDXGISwapChain1> swapChain;

	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		graphicQueue.GetCommandQueue(),
		Win32Application::GetHwnd(),
		&swapChainDesc,
		nullptr, nullptr,
		&swapChain), "Failed to create swap chain");

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER), "Failed to make window association");

	ThrowIfFailed(swapChain.As(&m_swapChain), "Failed to get swap chain");

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void SwapChain::Present(const uint32_t fence)
{
	// save the last fence used for this frame
	m_fenceValues[m_frameIndex] = fence;

	// Present the frame. vsync disabled
	ThrowIfFailed(m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING));

	// Update the frame index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	m_graphicQueue->WaitForFence(m_fenceValues[m_frameIndex]);
}

void SwapChain::Resize(const uint32_t width, const uint32_t height)
{
	for (auto & texture : m_backBufferTextures)
		texture.Destroy();

	DXGI_SWAP_CHAIN_DESC desc = {};
	SUCCEEDED(m_swapChain->GetDesc(&desc));
	ThrowIfFailed(m_swapChain->ResizeBuffers(BufferCount, width, height, desc.BufferDesc.Format, desc.Flags));

	CreateBackBufferTextures();
}

uint32_t SwapChain::GetCurrentBackBufferIndex() const
{
	return m_frameIndex;
}

void SwapChain::CreateBackBufferTextures()
{
	for (UINT n = 0; n < BufferCount; n++)
	{
		ComPtr<ID3D12Resource> backBuffer = nullptr;
		ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(backBuffer.GetAddressOf())), "Failed to get buffer");

		auto& backBufferTexture = m_backBufferTextures[n];
		backBufferTexture.Create(std::format(L"BackBuffer #{}", n), backBuffer, nullptr);
	}
}
