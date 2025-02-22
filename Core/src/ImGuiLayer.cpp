#include "pch.hpp"

#include "ImGuiLayer.hpp"
#include "imgui.h"
#include "Win32Application.hpp"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"

namespace Zenyth
{
	ImGuiLayer::ImGuiLayer(ID3D12Device *device, ID3D12CommandQueue* commandQueue)
	{
		m_resourceHeap.Create(device, L"ImGui Resource Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	 // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	  // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;		 // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;	   // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(Zenyth::Win32Application::GetHwnd());

		ImGui_ImplDX12_InitInfo init_info = {};
		init_info.Device = device;
		init_info.CommandQueue = commandQueue;
		init_info.NumFramesInFlight = Application::FrameCount;
		init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
		// Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
		// (current version of the backend will only allocate one descriptor, future versions will need to allocate more)
		init_info.SrvDescriptorHeap = m_resourceHeap.GetHeapPointer();
		init_info.UserData = &m_resourceHeap;
		init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* init_info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
		{
			auto* heap = static_cast<DescriptorHeap*>(init_info->UserData);
			const auto handle = heap->Alloc();
			*out_cpu_handle = handle.CPU();
			*out_gpu_handle = handle.GPU();
		};
		init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* init_info, const D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, const D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
		{
			auto* heap = static_cast<DescriptorHeap*>(init_info->UserData);
			const DescriptorHandle handle(cpu_handle, gpu_handle);
			heap->Free(handle);
		};
		ImGui_ImplDX12_Init(&init_info);
	}

	void ImGuiLayer::Begin() const
	{
		(void)this;

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::Render(ID3D12GraphicsCommandList *commandList) const
	{
		(void)this;

		ID3D12DescriptorHeap* ppHeaps[] = { m_resourceHeap.GetHeapPointer() };
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
	}

	void ImGuiLayer::End() const
	{
		(void)this;

		const ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void ImGuiLayer::OnWindowSizeChanged(uint32_t width, uint32_t height) const
	{
		(void)this;

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
	}


	ImGuiLayer::~ImGuiLayer()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		m_resourceHeap.Destroy();
	}
}
