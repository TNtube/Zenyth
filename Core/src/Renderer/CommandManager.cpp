#include "pch.hpp"
#include "Renderer/CommandManager.hpp"

#include "Renderer/Renderer.hpp"

namespace Zenyth
{
	void CommandManager::Create()
	{
		m_graphicsQueue.Create();
		m_copyQueue.Create();
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

		SUCCEEDED(Renderer::pDevice->CreateCommandList(1, type, *allocator, nullptr, IID_PPV_ARGS(list)));
		SUCCEEDED((*list)->SetName(L"CommandList"));
	}
}
