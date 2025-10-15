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
		~Renderer() = default;

		[[nodiscard]] ID3D12Device* GetDevice() const { return m_Device.Get(); };
		CommandManager& GetCommandManager() { return m_commandManager; }
		DescriptorHeap& GetResourceHeap() { return m_resourceHeap; }
		MaterialManager& GetMaterialManager() { return m_materialManager; }

	private:
		explicit Renderer(bool useWrapDevice);

		static void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);

	private:
		friend class Application;
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device = nullptr;

		CommandManager m_commandManager;
		DescriptorHeap m_resourceHeap;
		MaterialManager m_materialManager;
	};
}
