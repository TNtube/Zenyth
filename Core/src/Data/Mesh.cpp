#include "../../include/Data/Mesh.hpp"

#include "Renderer/Renderer.hpp"

namespace Zenyth
{
	// Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const std::string& name)
	// {
	// 	std::wstring wName(name.begin(), name.end());
	// 	m_vertexBuffer.Create(Renderer::pDevice.Get(), std::format(L"Vertex Buffer #{}", wName), vertices.size(), sizeof(Vertex), vertices.data());
	// 	m_indexBuffer .Create(Renderer::pDevice.Get(), std::format(L"Index buffer #{}", wName), indices.size(), sizeof(uint32_t), indices.data());
	// }

	Mesh::Mesh(std::string name)
		: m_name(std::move(name))
	{
	}

	Mesh::Mesh(std::vector<Submesh> submeshes, std::string name)
		: m_submeshes(std::move(submeshes))
	{
		if (!name.empty()) m_name = std::move(name);
	}

	void Mesh::ComputeTangents()
	{
		for (auto& submesh : m_submeshes)
		{
			submesh.ComputeTangents();
		}
	}

	void Mesh::PushSubmesh(Submesh submesh)
	{
		m_submeshes.emplace_back(std::move(submesh));
	}

	// void Mesh::Draw(ID3D12GraphicsCommandList *commandList) const
	// {
	// 	commandList->IASetVertexBuffers(0, 1, m_vertexBuffer.GetVBV());
	// 	commandList->IASetIndexBuffer(m_indexBuffer.GetIBV());
	//
	// 	commandList->DrawIndexedInstanced(m_indexBuffer.GetElementCount(), 1, 0, 0, 0);
	// }
}
