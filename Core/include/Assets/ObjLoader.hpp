#pragma once

#include "Data/Submesh.hpp"
#include "Renderer/SubmeshRenderer.hpp"

namespace Zenyth
{
	class ObjLoader
	{
	public:
		explicit ObjLoader(std::filesystem::path objPath);

		bool LoadData();
		[[nodiscard]] std::vector<SubmeshRenderer> GenerateRenderers() const;

	private:
		enum class TokenType
		{
			Invalid,
			ObjectName,
			GroupName,
			Position,
			Normal,
			Uv,
			FaceIndices
		};

		struct Token
		{
			TokenType type = TokenType::Invalid;
			std::vector<std::string> data {};
		};

		struct VertexIndices
		{
			uint32_t position = 0;
			uint32_t normal = 0;
			uint32_t uv = 0;
		};

		struct GroupData
		{
			std::vector<VertexIndices> vertices;
			std::vector<uint32_t> indices;
			std::string name;
		};

		static Token TokenizeLine(const std::string& line);

		static bool ParseFaceIndices(const Token& token, std::vector<VertexIndices>& inVertices, std::vector<uint32_t>& indices);
		static bool ParseVec(const Token& token, Vector3& vec);

		[[nodiscard]] Submesh ProcessGroup(const GroupData& group) const;

		std::filesystem::path m_objPath;
		std::vector<Submesh> m_meshes;

		std::vector<Vector3> m_positions;
		std::vector<Vector3> m_normals;
		std::vector<Vector2> m_uvs;
	};
}
