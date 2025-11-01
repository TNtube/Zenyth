#pragma once
#include <SimpleMath.h>

class Vertex
{
public:
	Vector3 position; // float pad0 = 1;
	Vector3 normal;   // float pad1 = 1;
	Vector3 tangent;  // float pad2 = 1;
	Vector2 uv;
};
