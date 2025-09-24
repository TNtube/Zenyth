#include "pch.hpp"
#include "Assets/ObjParser.hpp"

namespace Zenyth
{
	std::vector<uint32_t> SplitToInt(const std::string& entry, char sep)
	{
		std::stringstream iss(entry);
		std::string segment;
		std::vector<uint32_t> output;

		while(std::getline(iss, segment, sep))
		{
			if (segment.empty())
			{
				output.push_back(-1);
				continue;
			}
			output.push_back(std::stoi(segment));
		}

		return output;
	}

	ObjParser::ObjParser(std::filesystem::path objPath) : m_objPath(std::move(objPath))
	{
	}

	bool ObjParser::LoadData()
	{
		float min = std::numeric_limits<float>::max();
		float max = std::numeric_limits<float>::min();
		std::ifstream file(m_objPath);
		if (file.is_open()) {

			MeshData* currentMesh = &m_meshesData.emplace_back();
			std::vector<DirectX::XMFLOAT4> vertices;
			std::vector<DirectX::XMFLOAT4> normals;
			std::vector<DirectX::XMFLOAT2> uvs;

			std::string line;
			while (std::getline(file, line)) {

				std::istringstream iss(line);
				std::string type;
				std::string objectName;

				iss >> type;

				if (type == "v")
				{
					DirectX::XMFLOAT4& pos = vertices.emplace_back();
					iss >> pos.x >> pos.y >> pos.z;
					pos.w = 1;
				} else if (type == "vt")
				{
					DirectX::XMFLOAT2& uv = uvs.emplace_back();
					iss >> uv.x >> uv.y;
				} else if (type == "vn")
				{
					DirectX::XMFLOAT4& norm = normals.emplace_back();
					iss >> norm.x >> norm.y >> norm.z;
					norm.w = 1;
				} else if (type == "f")
				{
					std::string a, b, c;
					iss >> a >> b >> c;
					std::vector points = {c, b, a};
					for (const auto& point: points)
					{
						auto indexes = SplitToInt(point, '/');
						currentMesh->indices.push_back(currentMesh->vertices.size());
						auto& vertex = currentMesh->vertices.emplace_back();
						vertex.position = vertices[indexes[0]-1];
						vertex.uv = uvs[indexes[1]-1];
						vertex.normal = normals[indexes[2]-1];
					}

				} else if (type == "g")
				{
					auto& meshName = currentMesh->name;
					iss >> meshName;
					currentMesh = &m_meshesData.emplace_back();
					vertices.clear();
					uvs.clear();
					normals.clear();
				} else if (type == "o")
				{
					iss >> objectName;
					currentMesh->name = objectName;
				}
			}
			file.close();

			return true;
		}
		return false;
	}

	std::vector<Mesh> ObjParser::GenerateMeshes() const
	{
		std::vector<Mesh> meshes;
		for (const auto& meshData : m_meshesData)
		{
			meshes.emplace_back(meshData.vertices, meshData.indices, meshData.name);
		}

		return meshes;
	}
}
