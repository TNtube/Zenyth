#pragma once
#include "Renderer/Model/Vertex.hpp"
#include "Renderer/Model/Mesh.hpp"

namespace Zenyth
{

	class ObjParser
	{
	public:
		explicit ObjParser(std::filesystem::path objPath);
		bool LoadData();
		std::vector<Mesh> GenerateMeshes() const;

	private:
		struct MeshData
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			std::string name;
		};

		std::filesystem::path m_objPath;
		std::vector<MeshData> m_meshesData;
	};
}
