#pragma once

namespace Zenyth
{
	enum class LightType
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

		uint32_t						type;
		DirectX::SimpleMath::Vector2	coneAngles;
	};
}
