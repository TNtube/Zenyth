#pragma once
#include "stdafx.h"


inline void ThrowIfFailed(HRESULT hr, const char* msg = "")
{
	if (FAILED(hr))
	{
		throw std::exception(msg);
	}
}