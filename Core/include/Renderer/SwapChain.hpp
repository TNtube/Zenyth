#pragma once
#include "Renderer/Texture.hpp"
#include "Renderer/CommandQueue.hpp"

class SwapChain
{
public:
	SwapChain(uint32_t width, uint32_t height, CommandQueue& graphicQueue);
	DELETE_COPY_CTOR(SwapChain)
	DEFAULT_MOVE_CTOR(SwapChain)

	Texture& GetCurrentBackBufferTexture() { return m_backBufferTextures[m_frameIndex]; }
	void Present(uint32_t fence);

	void Resize(uint32_t width, uint32_t height);
	uint32_t GetCurrentBackBufferIndex() const;

	static constexpr UINT BufferCount = 3;

private:
	void CreateBackBufferTextures();

	CommandQueue* m_graphicQueue;

	std::vector<Texture> m_backBufferTextures;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;

	uint32_t m_frameIndex = 0;
	uint64_t m_fenceValues[BufferCount] {};
};
