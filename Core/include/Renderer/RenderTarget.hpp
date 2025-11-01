#pragma once
#include "Core.hpp"
#include "Texture.hpp"

class Texture;

namespace AttachmentPoint
{
	enum Type
	{
		Color0,
		Color1,
		Color2,
		Color3,
		Color4,
		Color5,
		Color6,
		Color7,
		DepthStencil,
		NumAttachmentPoints
	};
}

class RenderTarget
{
public:
	RenderTarget();
	DELETE_COPY_CTOR(RenderTarget)
	DEFAULT_MOVE_CTOR(RenderTarget)

	void AttachTexture( AttachmentPoint::Type attachmentPoint, std::unique_ptr<Texture> texture );
	[[nodiscard]] Texture* GetTexture( AttachmentPoint::Type attachmentPoint ) const;

	void Resize( uint32_t width, uint32_t height );

	[[nodiscard]] uint32_t GetWidth() const;
	[[nodiscard]] uint32_t GetHeight() const;

	[[nodiscard]] D3D12_VIEWPORT GetViewport(
		DirectX::SimpleMath::Vector2 scale = { 1.0f, 1.0f },
		DirectX::SimpleMath::Vector2 bias = { 0.0f, 0.0f },
		float minDepth = 0.0f, float maxDepth = 1.0f ) const;

	[[nodiscard]] const std::vector<std::unique_ptr<Texture>>& GetTextures() const;

	[[nodiscard]] D3D12_RT_FORMAT_ARRAY GetRenderTargetFormats() const;
	[[nodiscard]] DXGI_FORMAT GetDepthStencilFormat() const;
	[[nodiscard]] DXGI_SAMPLE_DESC GetSampleDesc() const;

	void Destroy() const
	{
		for (const auto& texture : m_textures)
		{
			if (texture) texture->Destroy();
		}
	}

private:
	std::vector<std::unique_ptr<Texture>> m_textures;
	uint32_t m_width;
	uint32_t m_height;
};