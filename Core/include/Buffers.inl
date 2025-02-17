#pragma once


namespace Zenyth
{
	template<std::unsigned_integral TIndex>
	void IndexBuffer<TIndex>::Create(ID3D12Device* device, const std::wstring& name)
	{
		assert(m_indices.size() > 0);
		Buffer::Create(device, name, m_indices.size(), sizeof(TIndex), m_indices.data());

		m_ibv.BufferLocation = m_buffer->GetGPUVirtualAddress();
		if constexpr (std::is_same_v<TIndex, uint16_t>)
			m_ibv.Format = DXGI_FORMAT_R16_UINT;
		else
			m_ibv.Format = DXGI_FORMAT_R32_UINT;
		m_ibv.SizeInBytes = m_bufferSize;
	}

	template<std::unsigned_integral TIndex>
	void IndexBuffer<TIndex>::Create(ID3D12Device* device, const std::wstring& name, const uint32_t numElements, const uint32_t elementSize, const void* initialData)
	{
		assert(numElements > 0);
		Buffer::Create(device, name, numElements, elementSize, initialData);

		m_ibv.BufferLocation = m_buffer->GetGPUVirtualAddress();
		if constexpr (std::is_same_v<TIndex, uint16_t>)
			m_ibv.Format = DXGI_FORMAT_R16_UINT;
		else
			m_ibv.Format = DXGI_FORMAT_R32_UINT;
		m_ibv.SizeInBytes = m_bufferSize;
	}

	template<std::unsigned_integral TIndex>
	void IndexBuffer<TIndex>::PushTriangle(TIndex v0, TIndex v1, TIndex v2)
	{
		m_indices.reserve(m_indices.size() + 3);
		m_indices.push_back(v0);
		m_indices.push_back(v1);
		m_indices.push_back(v2);
	}

	template<typename TVertex>
	void VertexBuffer<TVertex>::Create(ID3D12Device* device, const std::wstring& name)
	{
		assert(m_vertices.size() > 0);
		Buffer::Create(device, name, m_vertices.size(), sizeof(TVertex), m_vertices.data());

		m_vbv.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_vbv.StrideInBytes = m_elementSize;
		m_vbv.SizeInBytes = m_bufferSize;
	}

	template<typename TVertex>
	void VertexBuffer<TVertex>::Create(ID3D12Device* device, const std::wstring& name, const uint32_t numElements, const uint32_t elementSize, const void* initialData)
	{
		assert(numElements > 0);
		assert(m_vertices.size() == 0); // if vertices are set, most likely a mistake
		Buffer::Create(device, name, numElements, elementSize, initialData);

		m_vbv.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_vbv.StrideInBytes = m_elementSize;
		m_vbv.SizeInBytes = m_bufferSize;
	}

	template<typename TVertex>
	uint32_t VertexBuffer<TVertex>::PushVertex(const TVertex& vertice)
	{
		m_vertices.push_back(vertice);
		return m_vertices.size() - 1;
	}

	template<typename TConstant>
	void ConstantBuffer<TConstant>::Create(ID3D12Device* device, const std::wstring& name, DescriptorHeap& resourceHeap)
	{
		Buffer::Create(device, name, 1, (sizeof(TConstant) + 255) & ~255, nullptr);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_buffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = m_bufferSize;

		m_cbvHandle = resourceHeap.Alloc(1);
		m_pDevice->CreateConstantBufferView(&cbvDesc, m_cbvHandle.CPU());

		Map(&m_mappedData);
	}

	template<typename TConstant>
	void ConstantBuffer<TConstant>::Create(ID3D12Device* device, const std::wstring& name, DescriptorHeap& resourceHeap, const TConstant& initialData)
	{
		Buffer::Create(device, name, 1, (sizeof(TConstant) + 255) & ~255, nullptr);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_buffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = m_bufferSize;

		m_cbvHandle = resourceHeap.Alloc(1);
		m_pDevice->CreateConstantBufferView(&cbvDesc, m_cbvHandle.CPU());

		Map(&m_mappedData);
		SetData(initialData);
	}

	template<typename TConstant>
	void ConstantBuffer<TConstant>::SetData(const TConstant& data)
	{
		assert(m_mapped);
		memcpy(m_mappedData, &data, m_bufferSize);
	}
}
