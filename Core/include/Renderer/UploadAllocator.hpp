#pragma once
#include "Buffers.hpp"
#include "Core.hpp"


namespace Zenyth
{
	struct BufferView
	{
		BufferView(UploadBuffer* baseBuffer, const size_t offset, const size_t size)
			: buffer(baseBuffer), offset(offset),
			  size(size), data(baseBuffer->GetMappedData() + offset) {}

		BufferView() = default;

		UploadBuffer* buffer;
		size_t offset;
		size_t size;
		void* data;
	};

	class UploadAllocator
	{
		friend class UploadAllocatorPool;
	public:
		explicit UploadAllocator(const D3D12_COMMAND_LIST_TYPE type) : m_type(type) {};

		DELETE_COPY_CTOR(UploadAllocator)
		DEFAULT_MOVE_CTOR(UploadAllocator)

		~UploadAllocator() = default;

		BufferView Allocate(size_t size, size_t alignment = 256);
		void Return(uint64_t fenceValue, const BufferView &buffer);


	private:
		void FlushUsedViews();
		void MergeAvailableViews();

		struct View
		{
			size_t offset;
			size_t size;

			[[nodiscard]] bool IsValid() const { return size != 0; }
		};

		struct InUseView
		{
			uint64_t fenceValue;
			View view;
		};

		D3D12_COMMAND_LIST_TYPE m_type;
		UploadBuffer m_buffer;
		size_t m_offset = 0;
		std::vector<InUseView> m_inUseViews;
		std::vector<View> m_availableView;
		static constexpr size_t BUFFER_SIZE = 0x200000; // 2MB
	};

	class UploadAllocatorPool
	{
	public:
		explicit UploadAllocatorPool(const D3D12_COMMAND_LIST_TYPE type) : m_type(type) {};

		DELETE_COPY_CTOR(UploadAllocatorPool)
		DEFAULT_MOVE_CTOR(UploadAllocatorPool)

		~UploadAllocatorPool() = default;

		BufferView Allocate(size_t size, size_t alignment = 256);
		void Return(uint64_t fenceValue, const BufferView &buffer) const;

	private:
		D3D12_COMMAND_LIST_TYPE m_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		std::vector<std::unique_ptr<UploadAllocator>> m_allocators;
	};
}
