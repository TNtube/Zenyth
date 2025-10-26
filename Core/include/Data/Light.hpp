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
		Vector3		position;
		Vector3		direction;

		float		radiusSq;
		Vector3		color;

		LightType	type;
		Vector2		coneAngles;
	};
}
