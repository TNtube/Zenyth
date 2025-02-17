#pragma once
#include "Transform.hpp"
#include "Buffers.hpp"
#include "Block.hpp"
#include "Vertex.hpp"

struct ModelData
{
	Matrix model;
};

#define CHUNK_SIZE 16

class World;


class Chunk
{
public:
	Zenyth::Transform transform;

	Chunk();

	Chunk(World* world, Vector3 position);

	BlockId* GetBlock(Vector3 worldPosition);
	[[nodiscard]] const BlockId* GetBlock(Vector3 worldPosition) const;


	void Create(ID3D12Device* device);
	void Draw(ID3D12GraphicsCommandList* commandList, ShaderPass shaderPass) const;

private:
	friend class World;
	void AddFace(Vector3 position, Vector3 up, Vector3 right, Vector4 normal, Vector2 textCoord, ShaderPass shaderPass);

	[[nodiscard]] bool ShouldRenderFace(Vector3 position, Vector3 direction, const BlockData& data) const;

	World* m_world;
	ID3D12Device* m_device;

	bool m_hasBlocks = false;

	Zenyth::Buffer m_vertexBuffer[static_cast<int>(ShaderPass::Size)];
	std::vector<Vertex> m_vertices[static_cast<int>(ShaderPass::Size)];
	Zenyth::Buffer m_indexBuffer[static_cast<int>(ShaderPass::Size)];
	std::vector<uint32_t> m_indices[static_cast<int>(ShaderPass::Size)];

	Vector3 m_chunkPosition;

	std::vector<BlockId> m_blocks;
};
