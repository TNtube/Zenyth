#include "pch.hpp"
#include "Renderer/CommandManager.hpp"

#include "Application.hpp"
#include "Renderer/Renderer.hpp"

namespace Zenyth
{
	void CommandManager::Create(ID3D12Device* device)
	{
		m_graphicsQueue.Create(device);
		m_copyQueue.Create(device);
	}

	void CommandManager::Destroy()
	{
		m_graphicsQueue.Destroy();
		m_copyQueue.Destroy();
	}

	void CommandManager::GetNewCommandList(const D3D12_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList **list, ID3D12CommandAllocator **allocator)
	{
		switch (type)
		{
			case D3D12_COMMAND_LIST_TYPE_DIRECT:
				*allocator = m_graphicsQueue.AcquireAllocator();
				break;
			case D3D12_COMMAND_LIST_TYPE_COPY:
				*allocator = m_copyQueue.AcquireAllocator();
				break;
			default:
				assert(false);
				break;
		}

		auto& renderer = Application::Get().GetRenderer();
		SUCCEEDED(renderer.GetDevice()->CreateCommandList(1, type, *allocator, nullptr, IID_PPV_ARGS(list)));
		SUCCEEDED((*list)->SetName(L"CommandList"));
	}
}
