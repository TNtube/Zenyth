#pragma once
#include "Transform.hpp"
#include "Buffers.hpp"
#include "Block.hpp"
#include "Vertex.hpp"

struct ModelData
{
	DirectX::SimpleMath::Matrix model;
};

#define CHUNK_SIZE 16

class World;


class Chunk
{
public:

	Chunk();

	Chunk(World* world, DirectX::SimpleMath::Vector3 position);

	BlockId* GetBlock(DirectX::SimpleMath::Vector3 worldPosition);
	[[nodiscard]] const BlockId* GetBlock(DirectX::SimpleMath::Vector3 worldPosition) const;


	void Create(ID3D12Device *device, Zenyth::DescriptorHeap &resourceHeap);
	void Draw(ID3D12GraphicsCommandList* commandList, ShaderPass shaderPass) const;

private:
	friend class World;
	void AddFace(DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Vector3 up, DirectX::SimpleMath::Vector3 right, DirectX::SimpleMath::Vector4 normal, DirectX::SimpleMath::Vector2 textCoord, ShaderPass shaderPass);

	[[nodiscard]] bool ShouldRenderFace(DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Vector3 direction, const BlockData& data) const;

	World* m_world;
	ID3D12Device* m_device;

	bool m_hasBlocks[static_cast<int>(ShaderPass::Size)] = {false};

	Zenyth::VertexBuffer<Vertex> m_vertexBuffer[static_cast<int>(ShaderPass::Size)];
	Zenyth::IndexBuffer<uint32_t> m_indexBuffer[static_cast<int>(ShaderPass::Size)];
	Zenyth::ConstantBuffer<ModelData> m_constantBuffer; // shared between all passes

	DirectX::SimpleMath::Vector3 m_chunkPosition;

	std::vector<BlockId> m_blocks;
	Zenyth::Transform m_transform;
};
