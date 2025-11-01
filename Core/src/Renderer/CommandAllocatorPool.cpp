#include "pch.hpp"
#include "Renderer/CommandAllocatorPool.hpp"

#include "Application.hpp"
#include "Renderer/Renderer.hpp"


CommandAllocatorPool::CommandAllocatorPool(const D3D12_COMMAND_LIST_TYPE type) : m_type(type)
{
}

void CommandAllocatorPool::Clear()
{
	std::lock_guard lock(m_mutex);

	m_availableCommandAllocators = {};
	m_commandAllocators.clear();
}

ID3D12CommandAllocator * CommandAllocatorPool::AcquireAllocator(const uint64_t completedFenceValue)
{
	std::lock_guard lock(m_mutex);

	ID3D12CommandAllocator* outAllocator = nullptr;

	if (!m_availableCommandAllocators.empty())
	{
		auto& [fenceValue, availableAllocator] = m_availableCommandAllocators.front();

		// is the allocator still in use?
		if (fenceValue <= completedFenceValue)
		{
			outAllocator = availableAllocator;
			SUCCEEDED(outAllocator->Reset());
			m_availableCommandAllocators.pop();
		}
	}

	// no available allocator, create a new one
	if (!outAllocator)
	{
		const auto& renderer = Application::Get().GetRenderer();

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> newAllocator;
		ThrowIfFailed(renderer.GetDevice()->CreateCommandAllocator(m_type, IID_PPV_ARGS(&newAllocator)));
		outAllocator = newAllocator.Get();
		m_commandAllocators.push_back(newAllocator);
	}

	return outAllocator;
}

void CommandAllocatorPool::FreeAllocator(const uint64_t fenceValue, ID3D12CommandAllocator *commandAllocator)
{
	std::lock_guard lock(m_mutex);

	// we thrust the user to consistently reset the allocator
	m_availableCommandAllocators.push({ fenceValue, commandAllocator });
}
