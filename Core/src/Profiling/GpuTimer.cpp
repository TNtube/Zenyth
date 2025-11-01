#include "pch.hpp"
#include "Profiling/GpuTimer.hpp"

#include "Application.hpp"
#include "Renderer/CommandBatch.hpp"

namespace
{
	Microsoft::WRL::ComPtr<ID3D12QueryHeap> s_queryHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> s_readBackBuffer = nullptr;
	uint64_t* s_timeStampBuffer = nullptr;
	uint64_t s_fence = 0;
	uint32_t s_numTimers = 0;
	uint64_t s_validTimeStart = 0;
	uint64_t s_validTimeEnd = 0;
	double s_gpuTickDelta = 0.0;
}

constexpr uint32_t MaxTimer = 4096;

void GpuTimeManager::Init()
{
	auto& renderer = Application::Get().GetRenderer();
	const auto device = renderer.GetDevice();

	uint64_t gpuFrequency;
	renderer.GetCommandManager().GetGraphicsQueue().GetCommandQueue()->GetTimestampFrequency(&gpuFrequency);

	s_gpuTickDelta = 1.0 / static_cast<double>(gpuFrequency);

	D3D12_HEAP_PROPERTIES heapProps;
	heapProps.Type = D3D12_HEAP_TYPE_READBACK;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC bufferDesc;
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Alignment = 0;
	bufferDesc.Width = sizeof(uint64_t) * MaxTimer * 2;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(device->CreateCommittedResource( &heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&s_readBackBuffer) ));

	ThrowIfFailed(s_readBackBuffer->SetName(L"GpuTimeStamp Buffer"));

	D3D12_QUERY_HEAP_DESC QueryHeapDesc;
	QueryHeapDesc.Count = MaxTimer * 2;
	QueryHeapDesc.NodeMask = 1;
	QueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	ThrowIfFailed(device->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&s_queryHeap)));
	ThrowIfFailed(s_queryHeap->SetName(L"GpuTimeStamp QueryHeap"));
}

void GpuTimeManager::Destroy()
{
	s_readBackBuffer.Reset();
	s_queryHeap.Reset();
}

void GpuTimeManager::BeginReadback()
{
	auto& renderer = Application::Get().GetRenderer();
	renderer.GetCommandManager().WaitForFence(s_fence);

	D3D12_RANGE range;
	range.Begin = 0;
	range.End = (s_numTimers * 2) * sizeof(uint64_t);
	ThrowIfFailed(s_readBackBuffer->Map(0, &range, reinterpret_cast<void**>(&s_timeStampBuffer)));

	s_validTimeStart = s_timeStampBuffer[0];
	s_validTimeEnd = s_timeStampBuffer[1];

	// On the first frame, with random values in the timestamp query heap, we can avoid a misstart.
	if (s_validTimeEnd < s_validTimeStart)
	{
		s_validTimeStart = 0ull;
		s_validTimeEnd = 0ull;
	}
}

void GpuTimeManager::EndReadback()
{
	// Unmap with an empty range to indicate nothing was written by the CPU
	constexpr D3D12_RANGE emptyRange = {};
	s_readBackBuffer->Unmap(0, &emptyRange);
	s_timeStampBuffer = nullptr;

	auto batch = CommandBatch::Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);
	batch.InsertTimeStamp(s_queryHeap.Get(), 1);
	batch.ResolveTimeStamps(s_readBackBuffer.Get(), s_queryHeap.Get(), s_numTimers * 2);
	batch.InsertTimeStamp(s_queryHeap.Get(), 0);
	batch.InsertTimeStamp(s_queryHeap.Get(), 2);
	batch.InsertTimeStamp(s_queryHeap.Get(), 3);
	s_fence = batch.End();
}

float GpuTimeManager::GetTime(const uint32_t timer)
{
	assert(s_timeStampBuffer != nullptr && "Time stamp readback buffer is not mapped");
	assert(timer < s_numTimers && "Invalid GPU timer index");

	const uint64_t timeStamp1 = s_timeStampBuffer[timer * 2];
	const uint64_t timeStamp2 = s_timeStampBuffer[timer * 2 + 1];

	if (timeStamp1 < s_validTimeStart || timeStamp2 > s_validTimeEnd || timeStamp2 <= timeStamp1 )
		return 0.0f;

	return static_cast<float>(s_gpuTickDelta * (timeStamp2 - timeStamp1));
}

GpuTimer::GpuTimer()
{
	m_timer = s_numTimers++;
}

void GpuTimer::Begin(const CommandBatch& batch) const
{
	batch.InsertTimeStamp(s_queryHeap.Get(), m_timer * 2);
}

void GpuTimer::End(const CommandBatch& batch) const
{
	batch.InsertTimeStamp(s_queryHeap.Get(), m_timer * 2 + 1);
}
