#include "pch.hpp"
#include "Renderer/RenderTarget.hpp"

#include "Renderer/Texture.hpp"

namespace Zenyth
{
	RenderTarget::RenderTarget()
		: m_textures( AttachmentPoint::NumAttachmentPoints )
		, m_width( 0 ), m_height( 0 )
		{}

	void RenderTarget::AttachTexture(const AttachmentPoint::Type attachmentPoint, std::unique_ptr<Texture> texture )
	{
		m_textures[attachmentPoint] = std::move(texture);

		if ( texture && texture->GetResource() )
		{
			const auto desc = texture->GetResource()->GetDesc();

			m_width = static_cast<uint32_t>( desc.Width );
			m_height = static_cast<uint32_t>( desc.Height );
		}
	}

	Texture* RenderTarget::GetTexture(const AttachmentPoint::Type attachmentPoint ) const
	{
		return m_textures[attachmentPoint].get();
	}

	void RenderTarget::Resize(const uint32_t width, const uint32_t height )
	{
		m_width = width;
		m_height = height;
		for ( auto& texture: m_textures ) { if ( texture ) texture->Resize( width, height ); }
	}

	uint32_t RenderTarget::GetWidth() const
	{
		return m_width;
	}

	uint32_t RenderTarget::GetHeight() const
	{
		return m_height;
	}

	D3D12_VIEWPORT RenderTarget::GetViewport(
		const DirectX::SimpleMath::Vector2 scale,
		const DirectX::SimpleMath::Vector2 bias,
		const float minDepth, const float maxDepth ) const
	{
		uint64_t width  = 0;
		uint32_t height = 0;

		for ( int i = AttachmentPoint::Color0; i <= AttachmentPoint::Color7; ++i )
		{
			if (const auto& texture = m_textures[i] )
			{
				auto desc = texture->GetResource()->GetDesc();
				width = std::max( width, desc.Width );
				height = std::max( height, desc.Height );
			}
		}

		const D3D12_VIEWPORT viewport = {
			( width * bias.x ),    // TopLeftX
			( height * bias.y ),   // TopLeftY
			( width * scale.x ),   // Width
			( height * scale.y ),  // Height
			minDepth,              // MinDepth
			maxDepth               // MaxDepth
		};

		return viewport;
	}

	const std::vector<std::unique_ptr<Texture>>& RenderTarget::GetTextures() const
	{
		return m_textures;
	}

	D3D12_RT_FORMAT_ARRAY RenderTarget::GetRenderTargetFormats() const
	{
		D3D12_RT_FORMAT_ARRAY rtvFormats = {};

		for ( int i = AttachmentPoint::Color0; i <= AttachmentPoint::Color7; ++i )
		{
			if (const auto& texture = m_textures[i] )
			{
				rtvFormats.RTFormats[rtvFormats.NumRenderTargets++] = texture->GetResource()->GetDesc().Format;
			}
		}

		return rtvFormats;
	}

	DXGI_FORMAT RenderTarget::GetDepthStencilFormat() const
	{
		DXGI_FORMAT	dsvFormat = DXGI_FORMAT_UNKNOWN;
		if (const auto& depthStencilTexture = m_textures[AttachmentPoint::DepthStencil] )
		{
			dsvFormat = depthStencilTexture->GetResource()->GetDesc().Format;
		}

		return dsvFormat;
	}

	DXGI_SAMPLE_DESC RenderTarget::GetSampleDesc() const
	{
		DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };
		for ( int i = AttachmentPoint::Color0; i <= AttachmentPoint::Color7; ++i )
		{
			if (const auto& texture = m_textures[i] )
			{
				sampleDesc = texture->GetResource()->GetDesc().SampleDesc;
				break;
			}
		}

		return sampleDesc;
	}
}
