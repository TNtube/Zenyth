#pragma once
#include "Buffers.hpp"
#include "DescriptorHeap.hpp"

namespace Zenyth
{
	class Texture final : public GpuResource
	{
	public:
		Texture() = default;
		DELETE_COPY_CTOR(Texture)
		DELETE_MOVE_CTOR(Texture)

		void Create(std::wstring_view name, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue );
		void Create(std::wstring_view name, const Microsoft::WRL::ComPtr<ID3D12Resource>& resource, const D3D12_CLEAR_VALUE* clearValue );
		void Create(std::wstring_view name, int64_t height, int64_t width, bool sRGB, const void* initialData = nullptr);

		void Destroy() override;

		void Resize( uint32_t width, uint32_t height, uint32_t depthOrArraySize = 1 );

		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const;
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const;
		[[nodiscard]] DescriptorHandle GetSRV() const;
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetUAV( uint32_t mip ) const;

		[[nodiscard]] bool CheckRTVSupport() const;
		[[nodiscard]] bool CheckDSVSupport() const;
		[[nodiscard]] bool CheckSRVSupport() const;
		[[nodiscard]] bool CheckUAVSupport() const;

		static std::unique_ptr<Texture> LoadTextureFromFile(const char *filename, bool sRGB = true);
		static DXGI_FORMAT GetUAVCompatibleFormat( DXGI_FORMAT format );
		static bool IsSRGBFormat( DXGI_FORMAT format );

		static uint32_t loadedFile;
	private:
		void CreateViews();

		DescriptorHandle m_RTV;
		DescriptorHandle m_DSV;
		DescriptorHandle m_SRV;
		DescriptorHandle m_UAV;
	};


}
