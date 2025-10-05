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
		// Temporary arrays to accumulate tangent
		std::vector tan1(m_vertices.size(), Vector3::Zero);

		// Calculate tangents for each triangle
		for (size_t i = 0; i < m_indices.size(); i += 3) {
			const uint32_t i0 = m_indices[i + 0];
			const uint32_t i1 = m_indices[i + 1];
			const uint32_t i2 = m_indices[i + 2];

			const Vertex& v0 = m_vertices[i0];
			const Vertex& v1 = m_vertices[i1];
			const Vertex& v2 = m_vertices[i2];

			// Position deltas
			const Vector3 edge1 = v1.position - v0.position;
			const Vector3 edge2 = v2.position - v0.position;

			// UV deltas
			const float deltaU1 = v1.uv.x - v0.uv.x;
			const float deltaV1 = v1.uv.y - v0.uv.y;
			const float deltaU2 = v2.uv.x - v0.uv.x;
			const float deltaV2 = v2.uv.y - v0.uv.y;

			// Calculate tangent and bitangent
			float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

			// Avoid division by zero
			if (!isfinite(f)) {
				f = 1.0f;
			}

			const Vector3 tangent(
				f * (deltaV2 * edge1.x - deltaV1 * edge2.x),
				f * (deltaV2 * edge1.y - deltaV1 * edge2.y),
				f * (deltaV2 * edge1.z - deltaV1 * edge2.z)
			);

			// Accumulate for each vertex of the triangle
			tan1[i0] += tangent;
			tan1[i1] += tangent;
			tan1[i2] += tangent;
		}

		// Orthogonalize and normalize tangents using Gram-Schmidt
		for (size_t i = 0; i < m_vertices.size(); ++i) {
			Vector3 n = m_vertices[i].normal;
			Vector3 t = tan1[i];

			// Gram-Schmidt orthogonalize: t = normalize(t - n * dot(n, t))
			Vector3 tangent = t - n * n.Dot(t);
			tangent.Normalize();

			m_vertices[i].tangent = tangent;
		}
	}
}
