#include "pch.hpp"
#include "Assets/ObjLoader.hpp"

#include "Renderer/SubmeshRenderer.hpp"

namespace Zenyth
{
	using namespace DirectX::SimpleMath;
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

	ObjLoader::ObjLoader(std::filesystem::path objPath) : m_objPath(std::move(objPath))
	{
	}

	bool ObjLoader::LoadData()
	{
		std::ifstream file(m_objPath);
		if (file.is_open()) {

			// MeshData* currentMesh = &m_meshesData.emplace_back();
			GroupData currentGroup;

			std::string line;
			while (std::getline(file, line)) {
				auto token = TokenizeLine(line);
				Vector3 parsed {};

				switch (token.type)
				{
					case TokenType::Invalid:
						break;
					case TokenType::ObjectName:
						if (!currentGroup.name.empty())
							m_meshes.push_back(ProcessGroup(currentGroup));
						currentGroup = {};
						// currentObject.name = token.data[0];
						// currentGroup.name = token.data[0];
						break;
					case TokenType::GroupName:
						if (!currentGroup.name.empty())
							m_meshes.push_back(ProcessGroup(currentGroup));
						currentGroup = {};
						currentGroup.name = token.data[0];
						break;
					case TokenType::Position:
						if (!ParseVec(token, parsed)) return false;
						m_positions.push_back(parsed);
						break;
					case TokenType::Normal:
						if (!ParseVec(token, parsed)) return false;
						m_normals.push_back(parsed);
						break;
					case TokenType::Uv:
						if (!ParseVec(token, parsed)) return false;
						m_uvs.push_back({parsed.x, parsed.y});
						break;
					case TokenType::FaceIndices:
						std::vector<VertexIndices> vertices;
						std::vector<uint32_t> indices;

						if (!ParseFaceIndices(token, vertices, indices)) return false;

						int n = currentGroup.vertices.size();

						currentGroup.vertices.reserve(currentGroup.vertices.size() + std::distance(vertices.begin(),vertices.end()));
						currentGroup.vertices.insert(currentGroup.vertices.end(),vertices.begin(),vertices.end());

						currentGroup.indices.reserve(currentGroup.indices.size() + indices.size());
						for (int i : indices)
							currentGroup.indices.push_back(i + n);
						break;
				}
			}
			if (currentGroup.vertices.size())
				m_meshes.push_back(ProcessGroup(currentGroup));

			file.close();

			return true;
		}
		return false;
	}

	std::vector<SubmeshRenderer> ObjLoader::GenerateRenderers() const
	{
		std::vector<SubmeshRenderer> renderers;
		for (const auto& submesh : m_meshes)
			renderers.emplace_back(submesh);

		return renderers;
	}

	ObjLoader::Token ObjLoader::TokenizeLine(const std::string& line)
	{
		Token token;

		std::istringstream iss(line);
		std::string s;

		std::getline(iss, s, ' ');

		if (s == "o") token.type = TokenType::ObjectName;
		else if (s == "g") token.type = TokenType::GroupName;
		else if (s == "v") token.type = TokenType::Position;
		else if (s == "vn") token.type = TokenType::Normal;
		else if (s == "vt") token.type = TokenType::Uv;
		else if (s == "f") token.type = TokenType::FaceIndices;

		while (std::getline(iss, s, ' ')) {
			token.data.push_back(s);
		}

		if (token.data.empty())
			token.type = TokenType::Invalid;

		return token;
	}

	bool ObjLoader::ParseFaceIndices(const Token& token, std::vector<VertexIndices>& inVertices, std::vector<uint32_t>& indices)
	{
		inVertices.reserve(token.data.size());
		for (const auto& vertexIndices : token.data)
		{
			std::stringstream iss(vertexIndices);
			std::string segment;

			VertexIndices vertex;

			constexpr char sep = '/';

			if (!std::getline(iss, segment, sep))
				return false;
			vertex.position = std::stoi(segment) - 1;

			if (std::getline(iss, segment, sep) && !segment.empty())
				vertex.uv = std::stoi(segment) - 1;

			if (std::getline(iss, segment, sep) && !segment.empty())
				vertex.normal = std::stoi(segment) - 1;

			inVertices.push_back(vertex);
		}

		// triangulating the faces.
		const int n = inVertices.size();
		// We should have 2 triangle less than the number of vertices
		const int tc = n - 2;
		// tree indices per triangle.
		indices.reserve(tc * 3);

		for (int i = 0; i < tc; i++)
		{
			// from ccw to cw
			indices.push_back(0);
			indices.push_back(n - i - 1);
			indices.push_back(n - i - 2);
		}

		return true;
	}

	bool ObjLoader::ParseVec(const Token& token, Vector3& vec)
	{
		const auto size = token.data.size();

		if (size == 0 || size > 4)
			return false;

		vec = Vector3{1, 1, 1};

		vec.x = std::stof(token.data[0]);
		if (size > 1)
			vec.y = std::stof(token.data[1]);
		if (size > 2)
			vec.z = std::stof(token.data[2]);

		return true;
	}

	Submesh ObjLoader::ProcessGroup(const GroupData& group) const
	{
		Submesh output(group.name);

		output.GetIndices() = group.indices;
		output.GetVertices().reserve(group.vertices.size());

		for (const auto& [position, normal, uv] : group.vertices)
		{
			Vertex vertex { .position = m_positions[position] };

			if (normal < m_normals.size())
				vertex.normal = m_normals[normal];

			if (uv < m_uvs.size())
				vertex.uv = m_uvs[uv];

			output.GetVertices().push_back(vertex);
		}

		return output;
	}
}
