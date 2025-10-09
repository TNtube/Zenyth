#pragma once
#include "Core.hpp"
#include "MaterialDesc.hpp"
#include "Vertex.hpp"

namespace Zenyth
{
	class Submesh
	{
	public:
		Submesh() = default;
		explicit Submesh(std::string name);
		Submesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::string name = "");

		DELETE_COPY_CTOR(Submesh)
		DEFAULT_MOVE_CTOR(Submesh)

		[[nodiscard]] const std::string& GetName() const { return m_name; }

		[[nodiscard]] const std::vector<Vertex>& GetVertices() const { return m_vertices; }
		std::vector<Vertex>& GetVertices() { return m_vertices; }
		[[nodiscard]] const std::vector<uint32_t>& GetIndices() const { return m_indices; }
		std::vector<uint32_t>& GetIndices() { return m_indices; }

		[[nodiscard]] const MaterialDesc& GetMaterialDesc() const { return m_materialDesc; }
		MaterialDesc& GetMaterialDesc() { return m_materialDesc; }

		[[nodiscard]] std::size_t GetVertexCount() const { return m_vertices.size(); }
		[[nodiscard]] std::size_t GetIndexCount() const { return m_indices.size(); }

		void ComputeTangents();

	private:
		std::string m_name = "Default";
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;
		MaterialDesc m_materialDesc;
	};
}
