#pragma once

namespace zenyth
{
	class Texture
	{
	public:
		std::int32_t width, height, channels;
		std::vector<std::uint8_t> data;
	};
}