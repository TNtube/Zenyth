#pragma once
#include "Submesh.hpp"
#include "Vertex.hpp"


class Mesh
{
public:
	Mesh() = default;
	explicit Mesh(std::string name);
	explicit Mesh(std::vector<Submesh> submeshes, std::string name = "");

	void ComputeTangents();
	void PushSubmesh(Submesh submesh);

	static bool FromObjFile(const std::string& path, Mesh& output);
private:
	friend class MeshRenderer;
	std::string m_name = "DefaultMesh";
	std::vector<Submesh> m_submeshes;
	std::vector<MaterialDesc> m_materialDesc;
};
