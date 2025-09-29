#include "pch.hpp"

#include "Renderer/SubmeshRenderer.hpp"

#include "Renderer/Renderer.hpp"

namespace Zenyth
{
	SubmeshRenderer::SubmeshRenderer(const Submesh& submesh)
	{
		std::wstring wName(submesh.GetName().begin(), submesh.GetName().end());
		m_vertexBuffer.Create(Renderer::pDevice.Get(), std::format(L"Vertex Buffer #{}", wName), submesh.GetVertexCount(), sizeof(Vertex), submesh.GetVertices().data());
		m_indexBuffer .Create(Renderer::pDevice.Get(), std::format(L"Index buffer #{}", wName), submesh.GetIndexCount(), sizeof(uint32_t), submesh.GetIndices().data());
	}

	void SubmeshRenderer::Draw(ID3D12GraphicsCommandList* commandList) const
	{
		commandList->IASetVertexBuffers(0, 1, m_vertexBuffer.GetVBV());
		commandList->IASetIndexBuffer(m_indexBuffer.GetIBV());

		commandList->DrawIndexedInstanced(m_indexBuffer.GetElementCount(), 1, 0, 0, 0);
	}
}
