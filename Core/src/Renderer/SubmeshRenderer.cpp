#include "pch.hpp"

#include "Renderer/SubmeshRenderer.hpp"

#include "Application.hpp"
#include "Renderer/CommandBatch.hpp"
#include "Renderer/Renderer.hpp"

SubmeshRenderer::SubmeshRenderer(const Submesh& submesh)
{
	auto& renderer = Application::Get().GetRenderer();

	m_material = renderer.GetMaterialManager().GetMaterial(submesh.GetMaterialDesc());

	std::wstring wName(submesh.GetName().begin(), submesh.GetName().end());
	m_vertexBuffer.Create(std::format(L"Vertex Buffer #{}", wName), submesh.GetVertexCount(), sizeof(Vertex), submesh.GetVertices().data());
	m_indexBuffer .Create(std::format(L"Index buffer #{}", wName), submesh.GetIndexCount(), sizeof(uint32_t), submesh.GetIndices().data());
}

void SubmeshRenderer::Submit(CommandBatch& commandBatch) const
{
	commandBatch.SubmitMaterial(m_material);
	const auto commandList = commandBatch.GetCommandList();
	commandList->IASetVertexBuffers(0, 1, m_vertexBuffer.GetVBV());
	commandList->IASetIndexBuffer(m_indexBuffer.GetIBV());

	commandList->DrawIndexedInstanced(m_indexBuffer.GetElementCount(), 1, 0, 0, 0);
}
