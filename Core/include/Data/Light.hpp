#pragma once

namespace Zenyth
{
	enum class LightType : uint32_t
	{
		Point,
		Spot,
		Directional
	};


	struct LightData
	{
		DirectX::SimpleMath::Vector3	position;
		DirectX::SimpleMath::Vector3	direction;

		float							radiusSq;
		DirectX::SimpleMath::Vector3	color;

		LightType						type;
		DirectX::SimpleMath::Vector2	coneAngles;
	};
}
