#pragma
#include "stdafx.h"


void ThrowIfFailed(HRESULT hr, const char* msg = "")
{
	if (FAILED(hr))
	{
		throw std::exception(msg);
	}
}