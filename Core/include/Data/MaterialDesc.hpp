#pragma once
#include <DirectXColors.h>


namespace Zenyth
{
	using namespace DirectX::SimpleMath;

	struct MaterialDesc
	{
		std::string name;

		std::string vsPath = "resources/shaders/basic_vs.hlsl";
		std::string psPath = "resources/shaders/basic_ps.hlsl";

		Vector3 ambient { 1.0f };
		Vector3 diffuse { DirectX::Colors::Magenta };
		Vector3 specular {1.0f};
		float specularStrength = 1.0f;

		// ----- transparency handling, not used for the moment ------
		// float transparency;
		// Vector3 transmissionColor;

		std::string diffuseMap;
		std::string normalMap;
		std::string specularMap;

		bool operator==(const MaterialDesc& other) const = default;

		bool operator<(const MaterialDesc& other) const {
			if (name != other.name) return name < other.name;
			if (vsPath != other.vsPath) return vsPath < other.vsPath;
			if (psPath != other.psPath) return psPath < other.psPath;
			if (diffuseMap != other.diffuseMap) return diffuseMap < other.diffuseMap;
			if (normalMap != other.normalMap) return normalMap < other.normalMap;
			return specularMap < other.specularMap;
		}
	};
}
