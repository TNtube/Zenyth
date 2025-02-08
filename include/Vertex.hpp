#pragma once
#include "stdafx.h"


class Vertex
{
public:
	Vertex() = default;
	Vertex(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color)
		: position(position), color(color) {}

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};