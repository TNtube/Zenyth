#include "Data/Mesh.hpp"

#include "Renderer/Renderer.hpp"

namespace Zenyth
{
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
}
