#include "Renderer/Model/Mesh.hpp"

#include "Renderer/Renderer.hpp"

namespace Zenyth
{
	Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const std::string& name)
	{
		std::wstring wName(name.begin(), name.end());
		m_vertexBuffer.Create(Renderer::pDevice.Get(), std::format(L"Vertex Buffer #{}", wName), vertices.size(), sizeof(Vertex), vertices.data());
		m_indexBuffer .Create(Renderer::pDevice.Get(), std::format(L"Index buffer #{}", wName), indices.size(), sizeof(uint32_t), indices.data());
	}

	void Mesh::Draw(ID3D12GraphicsCommandList *commandList) const
	{
		commandList->IASetVertexBuffers(0, 1, m_vertexBuffer.GetVBV());
		commandList->IASetIndexBuffer(m_indexBuffer.GetIBV());

		commandList->DrawIndexedInstanced(m_indexBuffer.GetElementCount(), 1, 0, 0, 0);
	}
}
