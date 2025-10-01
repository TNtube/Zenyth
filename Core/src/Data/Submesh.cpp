#include "pch.hpp"

#include "Data/Submesh.hpp"

namespace Zenyth
{
	Submesh::Submesh(std::string name)
		: m_name(std::move(name))
	{
	}

	Submesh::Submesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::string name)
		: m_vertices(std::move(vertices)), m_indices(std::move(indices))
	{
		if (!name.empty()) m_name = std::move(name);

		ComputeTangents();
	}

	void Submesh::ComputeTangents()
	{
		for (int i = 0; i < GetIndexCount(); i += 3)
		{
			auto& v1 = m_vertices[m_indices[i]];
			auto& v2 = m_vertices[m_indices[i + 1]];
			auto& v3 = m_vertices[m_indices[i + 2]];

			const auto e1  = v2.position - v1.position;
			const auto e2 = v3.position - v1.position;

			const auto uvd1  = v2.uv - v1.uv;
			const auto uvd2 = v3.uv - v1.uv;

			const float denominator = uvd1.x * uvd2.y - uvd2.x * uvd2.y;

			Vector3 tangent {0, 0, 0};

			if (denominator != 0.f)
				tangent = (e1 * uvd2.y - e2 * uvd1.y) / denominator;

			v1.tangent += tangent;
			v2.tangent += tangent;
			v3.tangent += tangent;
		}

		for (Vertex& vert : m_vertices)
		{
			vert.tangent = vert.tangent - vert.normal * vert.tangent.Dot(vert.normal);
			vert.tangent.Normalize();
		}
	}
}
