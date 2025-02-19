#pragma once

#include "Chunk.hpp"
#include <array>

#define CHUNK_Y_COUNT 8

class World
{
public:
	void Generate(ID3D12Device* device, Zenyth::DescriptorHeap& resourceHeap);
	void Draw(ID3D12GraphicsCommandList* commandList, ShaderPass pass);

	Chunk* GetChunk(DirectX::SimpleMath::Vector3 chunkPosition);

	// whole chunk position
	static DirectX::SimpleMath::Vector3 WorldToChunkPosition(DirectX::SimpleMath::Vector3 worldPosition);
	// local block position in the chunk
	static DirectX::SimpleMath::Vector3 WorldToLocalPosition(DirectX::SimpleMath::Vector3 worldPosition);

private:
	static size_t HashChunkPosition(DirectX::SimpleMath::Vector3 position);

	std::unordered_map<size_t, std::array<Chunk, CHUNK_Y_COUNT>> m_chunks;
};
