#include "pch.hpp"

#include "Chunk.hpp"
using namespace DirectX::SimpleMath;

Vector4 ToVec4(Vector3 v)
{
	return {v.x, v.y, v.z, 1.0f};
}

void Chunk::AddFace(Vector3 position, Vector3 up, Vector3 right, Vector4 normal, Vector2 textCoord, ShaderPass shaderPass)
{
	float u = textCoord.x / 16.0f;
	float v = textCoord.y / 16.0f;
	constexpr float one = 1.0f / 16.0f;
	const int pass = static_cast<int>(shaderPass);

	auto a = m_vertexBuffer[pass].PushVertex({ToVec4(position), normal, {u, v + one}});
	auto b = m_vertexBuffer[pass].PushVertex({ToVec4(position + up), normal, {u, v}});
	auto c = m_vertexBuffer[pass].PushVertex({ToVec4(position+ right), normal, { u + one, v + one}});
	auto d = m_vertexBuffer[pass].PushVertex({ToVec4(position+ up + right), normal, {u + one, v}});

	m_indexBuffer[pass].PushTriangle(a, b, c);
	m_indexBuffer[pass].PushTriangle(c, b, d);
	m_hasBlocks[pass] = true;
}

bool Chunk::ShouldRenderFace(Vector3 position, Vector3 direction, const BlockData& data) const
{
	Vector3 next = {position.x + direction.x, position.y + direction.y, position.z + direction.z};

	// auto chunkPosition = World::WorldToChunkPosition(next);
	// auto chunk = chunkPosition == m_chunkPosition ? this : m_world->GetChunk(chunkPosition);


	auto chunk = this;
	if (next.x < 0 || next.y < 0 || next.z < 0 || next.x >= CHUNK_SIZE || next.y >= CHUNK_SIZE || next.z >= CHUNK_SIZE)
		chunk = nullptr;

	if (chunk == nullptr)
		return true;

	BlockId nextBlock = *chunk->GetBlock(next);
	const BlockData* nextData = &BlockData::Get(nextBlock);

	bool nextTransparent = (data.pass == ShaderPass::Opaque || data.id != nextData->id) && nextData->pass != ShaderPass::Opaque;
	return nextTransparent || nextBlock == EMPTY;
	return true;
}


Chunk::Chunk() : m_world(nullptr), m_device(nullptr), m_blocks(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, EMPTY) {
}

Chunk::Chunk(World* world, Vector3 position)
	: m_world(world), m_device(nullptr), m_chunkPosition(position),
	  m_blocks(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, EMPTY) {
}

BlockId* Chunk::GetBlock(const Vector3 worldPosition)
{
	const Vector3 localPosition = worldPosition - m_chunkPosition * CHUNK_SIZE;
	return &m_blocks[static_cast<int>(localPosition.x)
					+ static_cast<int>(localPosition.y) * CHUNK_SIZE
					+ static_cast<int>(localPosition.z) * CHUNK_SIZE * CHUNK_SIZE];
}
const BlockId* Chunk::GetBlock(const Vector3 worldPosition) const
{
	const Vector3 localPosition = worldPosition - m_chunkPosition * CHUNK_SIZE;
	return &m_blocks[static_cast<int>(localPosition.x)
					+ static_cast<int>(localPosition.y) * CHUNK_SIZE
					+ static_cast<int>(localPosition.z) * CHUNK_SIZE * CHUNK_SIZE];
}

void Chunk::Create(ID3D12Device* device, Zenyth::DescriptorHeap& resourceHeap)
{
	m_device = device;
	const bool hasBlocks = std::ranges::any_of(m_blocks, [](const BlockId id) { return id != EMPTY; });
	std::fill_n(m_hasBlocks, static_cast<int>(ShaderPass::Size), hasBlocks);
	if (!hasBlocks)
		return;

	std::fill_n(m_hasBlocks, static_cast<int>(ShaderPass::Size), false);

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (int i = 0; i < m_blocks.size(); i++)
	{
		const auto blockId = m_blocks[i];

		if (blockId == EMPTY) continue;

		Vector3 blockPosition = {
			static_cast<float>(i % CHUNK_SIZE),
			static_cast<float>((i / CHUNK_SIZE) % CHUNK_SIZE),
			static_cast<float>(i / (CHUNK_SIZE * CHUNK_SIZE))
		};

		const BlockData& data = BlockData::Get(blockId);

		const Vector2 topTexCoord{static_cast<float>(data.texIdTop % 16), static_cast<float>(data.texIdTop / 16)};
		const Vector2 sideTexCoord{static_cast<float>(data.texIdSide % 16), static_cast<float>(data.texIdSide / 16)};
		const Vector2 bottomTexCoord{static_cast<float>(data.texIdBottom % 16), static_cast<float>(data.texIdBottom / 16)};

		auto back =		Vector3{-0.5f, -0.5f,  0.5f};
		auto right =	Vector3{ 0.5f, -0.5f,  0.5f};
		auto front =	Vector3{ 0.5f, -0.5f, -0.5f};
		auto left =		Vector3{-0.5f, -0.5f, -0.5f};
		auto top =		Vector3{-0.5f,  0.5f,  0.5f};
		auto bottom =	Vector3{-0.5f, -0.5f, -0.5f};

		if (ShouldRenderFace(blockPosition, Vector3::Backward, data))
			AddFace(blockPosition + back, Vector3::Up, Vector3::Right, {0.0f, 0.0f, 1.0f, 1.0f}, sideTexCoord, data.pass);
		if (ShouldRenderFace(blockPosition, Vector3::Right, data))
			AddFace(blockPosition + right, Vector3::Up, Vector3::Forward, {1.0f, 0.0f, 0.0f, 1.0f}, sideTexCoord, data.pass);
		if (ShouldRenderFace(blockPosition, Vector3::Forward, data))
			AddFace(blockPosition + front, Vector3::Up, Vector3::Left, {0.0f, 0.0f, -1.0f, 1.0f}, sideTexCoord, data.pass);
		if (ShouldRenderFace(blockPosition, Vector3::Left, data))
			AddFace(blockPosition + left, Vector3::Up, Vector3::Backward, {-1.0f, 0.0f, 0.0f, 1.0f}, sideTexCoord, data.pass);
		if (ShouldRenderFace(blockPosition, Vector3::Up, data))
			AddFace(blockPosition + top, Vector3::Forward, Vector3::Right, {0.0f, 1.0f, 0.0f, 1.0f}, topTexCoord, data.pass);
		if (ShouldRenderFace(blockPosition, Vector3::Down, data))
			AddFace(blockPosition + bottom, Vector3::Backward, Vector3::Right, {0.0f, -1.0f, 0.0f, 1.0f}, bottomTexCoord, data.pass);

		// AddFace({-0.5f, -0.5f, 0.5f}, Vector3::Up, Vector3::Right, sideTexCoord);			/ front
		// AddFace({0.5f,  -0.5f, 0.5f}, Vector3::Up, Vector3::Forward, sideTexCoord);			// back
		// AddFace({0.5f,  -0.5f, -0.5f}, Vector3::Up, Vector3::Left, sideTexCoord);			// left
		// AddFace({-0.5f,  -0.5f, -0.5f}, Vector3::Up, Vector3::Backward, sideTexCoord);		// right
		// AddFace({-0.5f, 0.5f, 0.5f}, Vector3::Forward, Vector3::Right, topTexCoord);			// top
		// AddFace({-0.5f,  -0.5f, -0.5f}, Vector3::Backward, Vector3::Right, bottomTexCoord);	// down
	}

	int passes = 0;

	for (int i = 0; i < static_cast<int>(ShaderPass::Size); ++i)
	{
		if (!m_hasBlocks[i])
			continue;
		m_vertexBuffer[i].Create(device, std::format(L"Vertex Buffer #{}", i));
		m_indexBuffer[i].Create(device, std::format(L"Index buffer #{}", i));
		passes++;
	}

	if (passes > 0)
	{
		m_constantBuffer.Create(device, L"Model Constant Buffer", resourceHeap);
		m_constantBuffer.SetData({m_transform.GetTransformMatrix().Transpose()});
	}
}

void Chunk::Draw(ID3D12GraphicsCommandList* commandList, ShaderPass shaderPass) const {
	const int pass = static_cast<int>(shaderPass);
	if (!m_hasBlocks[pass])
		return;

	commandList->IASetVertexBuffers(0, 1, m_vertexBuffer[pass].GetVertexBufferView());
	commandList->IASetIndexBuffer(m_indexBuffer[pass].GetIndexBufferView());

	commandList->SetGraphicsRootDescriptorTable(1, m_constantBuffer.GetDescriptorHandle().GPU());

	commandList->DrawIndexedInstanced(m_indexBuffer[pass].GetElementCount(), 1, 0, 0, 0);
}
