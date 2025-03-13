#pragma once

namespace Math
{
	template <typename T>
	T AlignToMask(T size, T alignment) {
		return static_cast<T>(size + (alignment - 1) & ~(alignment - 1));
	}
}