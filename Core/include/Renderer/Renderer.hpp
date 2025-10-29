#pragma once
#include "CommandManager.hpp"
#include "Material.hpp"

namespace Zenyth
{
	class Renderer final
	{
	public:
		DELETE_COPY_CTOR(Renderer)
		DELETE_MOVE_CTOR(Renderer)

		void Init();
		~Renderer();


		[[nodiscard]] ID3D12Device* GetDevice() const { return m_device.Get(); };
		CommandManager& GetCommandManager() { return m_commandManager; }
		MaterialManager& GetMaterialManager() { return m_materialManager; }

		const DescriptorHeap& GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) { return m_descriptorHeaps[type]; }

		DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, int64_t count = 1);
		void FreeDescriptor(const DescriptorHandle& handle, int64_t count = 1);

		static constexpr uint8_t FrameCount = 3;

	private:
		explicit Renderer(bool useWrapDevice);

		static void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);

	private:
		friend class Application;
		Microsoft::WRL::ComPtr<ID3D12Device> m_device = nullptr;

		CommandManager m_commandManager;

		DescriptorHeap m_descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
		{
			DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
			DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),
			DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV),
			DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
		};

		MaterialManager m_materialManager;
	};
}
