#pragma once

#include "Concepts.hpp"

namespace Zenyth::Math
{
	template <typename T>
	T AlignToMask(T size, T alignment) {
		return static_cast<T>(size + (alignment - 1) & ~(alignment - 1));
	}

	inline DirectX::SimpleMath::Matrix ApplyReverseZ(const DirectX::SimpleMath::Matrix& proj)
	{
		DirectX::SimpleMath::Matrix reverseZ = DirectX::SimpleMath::Matrix::Identity;
		reverseZ._33 = -1.0f;
		reverseZ._43 = 1.0f;
		reverseZ._34 = 0.0f;

		return proj * reverseZ;
	}

	template<floating_point T>
	constexpr bool IsAlmostEqual(T l, T r)
	{
		return r == std::nextafter(l, r);
	}
}
