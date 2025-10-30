#include "pch.hpp"

#include "Renderer/Texture.hpp"
#include "DDSTextureLoader.h"
#include "DirectXHelpers.h"
#include "ResourceUploadBatch.h"
#include "Renderer/CommandBatch.hpp"
#include "Renderer/Renderer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "Application.hpp"
#include "stb_image.h"

namespace
{
	DXGI_FORMAT GetUAVFormat( DXGI_FORMAT defaultFormat )
	{
		switch (defaultFormat)
		{
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
				return DXGI_FORMAT_R8G8B8A8_UNORM;

			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
				return DXGI_FORMAT_B8G8R8A8_UNORM;

			case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			case DXGI_FORMAT_B8G8R8X8_UNORM:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
				return DXGI_FORMAT_B8G8R8X8_UNORM;

			case DXGI_FORMAT_R32_TYPELESS:
			case DXGI_FORMAT_R32_FLOAT:
				return DXGI_FORMAT_R32_FLOAT;

#ifndef NDEBUG
			case DXGI_FORMAT_R32G8X24_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			case DXGI_FORMAT_D32_FLOAT:
			case DXGI_FORMAT_R24G8_TYPELESS:
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
			case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			case DXGI_FORMAT_D16_UNORM:

				assert(false && "Requested a UAV Format for a depth stencil Format.");
#endif

			default:
				return defaultFormat;
		}
	}
	D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDesc(const D3D12_RESOURCE_DESC& resDesc, UINT mipSlice)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format						   = GetUAVFormat(resDesc.Format);

		switch ( resDesc.Dimension )
		{
		case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			if ( resDesc.DepthOrArraySize > 1 )
			{
				uavDesc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
				uavDesc.Texture1DArray.ArraySize       = resDesc.DepthOrArraySize - 0;
				uavDesc.Texture1DArray.FirstArraySlice = 0;
				uavDesc.Texture1DArray.MipSlice        = mipSlice;
			}
			else
			{
				uavDesc.ViewDimension      = D3D12_UAV_DIMENSION_TEXTURE1D;
				uavDesc.Texture1D.MipSlice = mipSlice;
			}
			break;
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			if ( resDesc.DepthOrArraySize > 1 )
			{
				uavDesc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				uavDesc.Texture2DArray.ArraySize       = resDesc.DepthOrArraySize - 0;
				uavDesc.Texture2DArray.FirstArraySlice = 0;
				uavDesc.Texture2DArray.PlaneSlice      = 0;
				uavDesc.Texture2DArray.MipSlice        = mipSlice;
			}
			else
			{
				uavDesc.ViewDimension        = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.PlaneSlice = 0;
				uavDesc.Texture2D.MipSlice   = mipSlice;
			}
			break;
		case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			uavDesc.ViewDimension         = D3D12_UAV_DIMENSION_TEXTURE3D;
			uavDesc.Texture3D.WSize       = resDesc.DepthOrArraySize - 0;
			uavDesc.Texture3D.FirstWSlice = 0;
			uavDesc.Texture3D.MipSlice    = mipSlice;
			break;
		default:
			throw std::exception( "Invalid resource dimension." );
		}

		return uavDesc;
	}


	std::unordered_map<std::string, std::shared_ptr<Zenyth::Texture>> s_textureCache;
	std::array<std::shared_ptr<Zenyth::Texture>, Zenyth::DefaultTexture::Size> s_defaultTextures;
}

namespace Zenyth
{
	uint32_t Texture::loadedFile = 0;

	void Texture::CreateViews()
	{
		auto& renderer = Application::Get().GetRenderer();
		const auto device = renderer.GetDevice();
		const CD3DX12_RESOURCE_DESC desc( m_resource->GetDesc() );

		if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0 && CheckRTVSupport())
		{
			if (m_RTV.IsNull())
				m_RTV = renderer.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			device->CreateRenderTargetView(m_resource.Get(), nullptr, m_RTV.CPU());
		}

		if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
			depthStencilDesc.Format = desc.Format == DXGI_FORMAT_R32_TYPELESS ? DXGI_FORMAT_D32_FLOAT : desc.Format;
			depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

			if (m_DSV.IsNull())
				m_DSV = renderer.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			device->CreateDepthStencilView(m_resource.Get(), &depthStencilDesc, m_DSV.CPU());
		}

		if ((desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) == 0)
		{

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = desc.Format == DXGI_FORMAT_R32_TYPELESS ? DXGI_FORMAT_R32_FLOAT : desc.Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = desc.MipLevels;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			if (m_SRV.IsNull())
				m_SRV = renderer.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_SRV.CPU());
		}

		if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0 && desc.DepthOrArraySize == 1)
		{
			if (m_UAV.IsNull())
				m_UAV = renderer.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, desc.MipLevels);

			for ( int i = 0; i < desc.MipLevels; ++i )
			{
				auto uavDesc = GetUAVDesc(desc, i);
				device->CreateUnorderedAccessView(m_resource.Get(), nullptr, &uavDesc, m_UAV.CPU(i));
			}
		}
	}

	void Texture::Create(
		const std::wstring_view name,
		const D3D12_RESOURCE_DESC& resourceDesc,
		const D3D12_CLEAR_VALUE* clearValue)
	{
		GpuResource::Create(resourceDesc, clearValue);
		m_resource->SetName(name.data());
		CreateViews();
	}

	void Texture::Create(
		const std::wstring_view name,
		const Microsoft::WRL::ComPtr<ID3D12Resource>& resource,
		const D3D12_CLEAR_VALUE* clearValue)
	{
		GpuResource::Create(resource, clearValue);
		m_resource->SetName(name.data());
		CreateViews();
	}

	void Texture::Create(const std::wstring_view name, const int64_t width, const int64_t height, const bool sRGB, const void* initialData)
	{
		m_UsageState = D3D12_RESOURCE_STATE_COPY_DEST;

		// Create texture resource
		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
			sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
			width,
			height,
			1 );

		desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		GpuResource::Create(desc, nullptr);

		m_resource->SetName(name.data());

		CreateViews();

		if (initialData)
		{
			D3D12_SUBRESOURCE_DATA subresource;
			subresource.pData = initialData;
			subresource.RowPitch = width * 4;
			subresource.SlicePitch = subresource.RowPitch * height;

			CommandBatch::InitializeTexture(*this, 1u, &subresource);

			if (m_resource->GetDesc().MipLevels > 1) // hardcoded 1 because we only have one subresource atm
			{
				auto batch = CommandBatch::Begin(D3D12_COMMAND_LIST_TYPE_COMPUTE);
				batch.GenerateMips(*this);
				batch.End();
			}

			auto batch = CommandBatch::Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);
			batch.TransitionBarrier(*this, D3D12_RESOURCE_STATE_GENERIC_READ, true);
			batch.End();
		}
	}

	void Texture::Destroy()
	{
		if (!m_RTV.IsNull())
			Application::Get().GetRenderer().FreeDescriptor(m_RTV);
		if (!m_SRV.IsNull())
			Application::Get().GetRenderer().FreeDescriptor(m_SRV);
		if (!m_UAV.IsNull())
			Application::Get().GetRenderer().FreeDescriptor(m_UAV, m_resource->GetDesc().MipLevels);
		if (!m_DSV.IsNull())
			Application::Get().GetRenderer().FreeDescriptor(m_DSV);

		GpuResource::Destroy();
	}

	void Texture::Resize(const uint32_t width, const uint32_t height, const uint32_t depthOrArraySize)
	{
		if ( m_resource )
		{
			CD3DX12_RESOURCE_DESC resDesc( m_resource->GetDesc() );

			resDesc.Width            = std::max( width, 1u );
			resDesc.Height           = std::max( height, 1u );
			resDesc.DepthOrArraySize = depthOrArraySize;
			resDesc.MipLevels        = resDesc.SampleDesc.Count > 1 ? 1 : 0;

			GpuResource::Create(resDesc, m_clearValue.get());

			CreateViews();
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetRTV() const
	{
		return m_RTV.CPU();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetDSV() const
	{
		return m_DSV.CPU();
	}

	DescriptorHandle Texture::GetSRV() const
	{
		return m_SRV;
	}

	DescriptorHandle Texture::GetUAV() const
	{
		return m_UAV;
	}

	bool Texture::CheckRTVSupport() const
	{
		return CheckFormatSupport( D3D12_FORMAT_SUPPORT1_RENDER_TARGET );
	}

	bool Texture::CheckDSVSupport() const
	{
		return CheckFormatSupport( D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL );
	}

	bool Texture::CheckSRVSupport() const
	{
		return CheckFormatSupport( D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE );
	}

	bool Texture::CheckUAVSupport() const
	{
		return CheckFormatSupport( D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW )
			&& CheckFormatSupport( D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD )
			&& CheckFormatSupport( D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE );
	}

	std::unique_ptr<Texture> Texture::LoadTextureFromFile(const char *filename, const bool sRGB)
	{
		auto output = std::make_unique<Texture>();

		int image_width = 0;
		int image_height = 0;
		int channel_count = 0;

		unsigned char* data = stbi_load(filename, &image_width, &image_height, &channel_count, 4);
		if (data == nullptr)
			return nullptr;

		const auto len = strlen(filename);
		const std::wstring wName(filename, filename + len);

		output->Create(wName, image_width, image_height, sRGB, data);

		stbi_image_free(data);

		loadedFile++;

		return output;
	}

	DXGI_FORMAT Texture::GetUAVCompatibleFormat(const DXGI_FORMAT format)
	{
		DXGI_FORMAT uavFormat = format;

		switch ( format )
		{
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8X8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
				uavFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
				break;
			case DXGI_FORMAT_R32_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT:
				uavFormat = DXGI_FORMAT_R32_FLOAT;
				break;
			default:
				break;
		}

		return uavFormat;
	}

	bool Texture::IsSRGBFormat(const DXGI_FORMAT format)
	{
		switch ( format )
		{
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				return true;
			default:
				return false;
		}
	}

	DXGI_FORMAT Texture::GetSRGBFormat( DXGI_FORMAT format )
	{
		DXGI_FORMAT srgbFormat = format;
		switch ( format )
		{
			case DXGI_FORMAT_R8G8B8A8_UNORM:
				srgbFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
				break;
			case DXGI_FORMAT_BC1_UNORM:
				srgbFormat = DXGI_FORMAT_BC1_UNORM_SRGB;
				break;
			case DXGI_FORMAT_BC2_UNORM:
				srgbFormat = DXGI_FORMAT_BC2_UNORM_SRGB;
				break;
			case DXGI_FORMAT_BC3_UNORM:
				srgbFormat = DXGI_FORMAT_BC3_UNORM_SRGB;
				break;
			case DXGI_FORMAT_B8G8R8A8_UNORM:
				srgbFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
				break;
			case DXGI_FORMAT_B8G8R8X8_UNORM:
				srgbFormat = DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
				break;
			case DXGI_FORMAT_BC7_UNORM:
				srgbFormat = DXGI_FORMAT_BC7_UNORM_SRGB;
				break;
			default:
				return srgbFormat;
		}

		return srgbFormat;
	}

	void TextureManager::ClearTextures()
	{
		s_textureCache.clear();
	}

	std::shared_ptr<Texture> TextureManager::GetTexture(const std::string& name, const bool sRGB)
	{
		auto& texture = s_textureCache[name];

		if (!texture)
			texture = Texture::LoadTextureFromFile(name.c_str(), sRGB);

		return texture;
	}

	std::shared_ptr<Texture> TextureManager::GetDefault(const DefaultTexture::Type texture)
	{
		if (auto def = s_defaultTextures[texture])
			return def;

		constexpr uint32_t white = 0xFFFFFFFF;
		s_defaultTextures[DefaultTexture::White] = std::make_shared<Texture>();
		s_defaultTextures[DefaultTexture::White]->Create(L"Default White", 1, 1, true, &white);

		constexpr uint32_t black = 0xFF000000;
		s_defaultTextures[DefaultTexture::Black] = std::make_shared<Texture>();
		s_defaultTextures[DefaultTexture::Black]->Create(L"Default Black", 1, 1, true, &black);

		constexpr uint32_t magenta = 0xFFFF00FF;
		s_defaultTextures[DefaultTexture::Magenta] = std::make_shared<Texture>();
		s_defaultTextures[DefaultTexture::Magenta]->Create(L"Default Magenta", 1, 1, true, &magenta);

		constexpr uint32_t normal = 0x00FF8080;
		s_defaultTextures[DefaultTexture::Normal] = std::make_shared<Texture>();
		s_defaultTextures[DefaultTexture::Normal]->Create(L"Default Normal", 1, 1, false, &normal);

		return s_defaultTextures[texture];
	}
}
