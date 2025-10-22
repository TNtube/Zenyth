#include "Data/Mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

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

	bool Mesh::FromObjFile(const std::string& path, Mesh& output)
	{
		tinyobj::ObjReaderConfig config;

		const auto pos = path.find_last_of("\\/");

		std::string directory = path.substr(0, pos + 1);

		tinyobj::ObjReader reader;

		if (!reader.ParseFromFile(path, config)) {
			if (!reader.Error().empty()) {
					std::cerr << "TinyObjReader: " << reader.Error();
			}
			exit(1);
		}

		if (!reader.Warning().empty()) {
			std::cout << "TinyObjReader: " << reader.Warning();
		}

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();


		std::vector<Submesh> submeshes;

		// Loop over shapes
		for (const auto & shape : shapes) {
			// Loop over faces(submeshes)
			size_t index_offset = 0;

			auto& currentSubmesh = submeshes.emplace_back(shape.name);
			currentSubmesh.GetVertices().reserve(shape.mesh.indices.size());
			currentSubmesh.GetIndices().reserve(shape.mesh.indices.size());

			uint32_t lastIndex = 0;

			for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
				// should always be 3
				const auto fv = static_cast<size_t>(shape.mesh.num_face_vertices[f]);

				currentSubmesh.GetIndices().push_back(lastIndex);
				currentSubmesh.GetIndices().push_back(lastIndex + 2);
				currentSubmesh.GetIndices().push_back(lastIndex + 1);
				lastIndex += fv;

				for (size_t v = 0; v < fv; v++) {
					Vertex vertex;

					tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
					const auto vi = idx.vertex_index;
					vertex.position.x = attrib.vertices[3*vi+0];
					vertex.position.y = attrib.vertices[3*vi+1];
					vertex.position.z = attrib.vertices[3*vi+2];

					if (idx.normal_index >= 0) {
						const auto ni = idx.normal_index;
						vertex.normal.x = attrib.normals[3*ni+0];
						vertex.normal.y = attrib.normals[3*ni+1];
						vertex.normal.z = attrib.normals[3*ni+2];
					}

					if (idx.texcoord_index >= 0) {
						const auto ti = idx.texcoord_index;
						vertex.uv.x = attrib.texcoords[2*ti+0];
						vertex.uv.y = 1.0f - attrib.texcoords[2*ti+1];
					}

					currentSubmesh.GetVertices().push_back(vertex);
				}

				const tinyobj::material_t& mat = materials[shape.mesh.material_ids[f]];
				auto& matDesc = currentSubmesh.GetMaterialDesc();

				matDesc.name = mat.name;
				// matDesc.ambient = Vector3{1, 1, 1};
				// matDesc.diffuse = Vector3{1, 1, 1};
				// matDesc.specular = Vector3{1, 1, 1};

				memcpy(&matDesc.ambient, mat.ambient, sizeof(Vector3));
				memcpy(&matDesc.diffuse, mat.diffuse, sizeof(Vector3));
				memcpy(&matDesc.specular, mat.specular, sizeof(Vector3));

				index_offset += fv;

				matDesc.diffuseMap  = std::string(directory).append(mat.diffuse_texname);
				matDesc.normalMap   = std::string(directory).append(mat.normal_texname);
				matDesc.specularMap = std::string(directory).append(mat.specular_highlight_texname);
			}
		}


		output.m_name = path.substr(pos + 1);
		output.m_submeshes = std::move(submeshes);

		output.ComputeTangents();

		return true;
	}
}
