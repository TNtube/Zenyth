#include "pch.hpp"

#include "Renderer/UploadAllocator.hpp"

#include "Application.hpp"
#include "Math/Utils.hpp"
#include "Renderer/Renderer.hpp"

namespace Zenyth
{
	BufferView UploadAllocator::Allocate(size_t size, const size_t alignment)
	{
		size = Math::AlignToMask(size, alignment);

		if (!m_buffer.IsValid())
		{
			m_buffer.Create(L"UploadAllocator", std::max(BUFFER_SIZE, size));
			m_buffer.Map();
		}

		FlushUsedViews();
		MergeAvailableViews();

		View view {};
		for (auto it = m_availableView.begin(); it != m_availableView.end(); ++it)
		{
			if (auto& availableView = *it; availableView.size >= size)
			{
				view = availableView;
				if (view.size == size)
				{
					// we can remove the view, it's not available anymore
					m_availableView.erase(it);
				}
				else
				{
					// shrink the view, minimum view size should be equal to alignment
					view.size = size;
					availableView.size -= size;
					availableView.offset += size;
				}
				break;
			}
		}

		if (view.IsValid())
		{
			return { &m_buffer, view.offset, size };
		}

		if (m_offset + size <= m_buffer.GetBufferSize())
		{
			auto offset = m_offset;
			m_offset += size;
			return { &m_buffer, offset, size };
		}

		return {};
	}

	void UploadAllocator::Return(uint64_t fenceValue, const BufferView &buffer)
	{
		m_inUseViews.emplace_back(fenceValue, View{buffer.offset, buffer.size});
	}

	void UploadAllocator::FlushUsedViews()
	{
		auto& renderer = Application::Get().GetRenderer();
		std::erase_if(m_inUseViews, [this, &renderer](const auto& inUseView) {
			if (renderer.GetCommandManager().GetQueue(m_type).IsFenceComplete(inUseView.fenceValue))
			{
				m_availableView.emplace_back(inUseView.view);
				return true;
			}
			return false;
		});
	}

	void UploadAllocator::MergeAvailableViews()
	{
		std::ranges::sort(m_availableView, [](const View& a, const View& b)
		{
			return a.offset < b.offset;
		});

		for (auto it = m_availableView.begin(); it != m_availableView.end();)
		{
			auto next = std::next(it);

			if (next != m_availableView.end() && it->offset + it->size == next->offset)
			{
				it->size += next->size;
				it = m_availableView.erase(next);
				continue;
			}

			++it;
		}
	}

	BufferView UploadAllocatorPool::Allocate(const size_t size, const size_t alignment)
	{
		for (const auto& allocator: m_allocators)
		{
			const auto bufferView = allocator->Allocate(size, alignment);
			if (bufferView.size != 0) return bufferView;
		}

		auto allocator = std::make_unique<UploadAllocator>(m_type);
		const auto view = allocator->Allocate(size, alignment);
		m_allocators.push_back(std::move(allocator));

		return view;
	}

	void UploadAllocatorPool::Return(const uint64_t fenceValue, const BufferView& buffer) const
	{
		for (auto& allocator: m_allocators)
		{
			if (buffer.buffer == &allocator->m_buffer)
			{
				allocator->Return(fenceValue, buffer);
				return;
			}
		}

		assert(false && "Buffer not found in any allocator");
	}
}
