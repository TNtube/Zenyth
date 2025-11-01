#include "pch.hpp"
#include "Renderer/CommandManager.hpp"

#include "Application.hpp"
#include "Renderer/Renderer.hpp"


void CommandManager::Create(ID3D12Device* device)
{
	m_graphicsQueue.Create(device);
	m_copyQueue.Create(device);
	m_computeQueue.Create(device);
}

void CommandManager::Destroy()
{
	IdleGPU();
	m_graphicsQueue.Destroy();
	m_copyQueue.Destroy();
	m_computeQueue.Destroy();
}

bool CommandManager::IsFenceComplete(const uint64_t fenceValue)
{
	return GetQueue(static_cast<D3D12_COMMAND_LIST_TYPE>(fenceValue >> 56)).IsFenceComplete(fenceValue);
}

void CommandManager::WaitForFence(const uint64_t fenceValue)
{
	GetQueue(static_cast<D3D12_COMMAND_LIST_TYPE>(fenceValue >> 56)).WaitForFence(fenceValue);
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
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			*allocator = m_computeQueue.AcquireAllocator();
			break;
		default:
			assert(false);
			break;
	}

	auto& renderer = Application::Get().GetRenderer();
	SUCCEEDED(renderer.GetDevice()->CreateCommandList(1, type, *allocator, nullptr, IID_PPV_ARGS(list)));
	SUCCEEDED((*list)->SetName(L"CommandList"));
}