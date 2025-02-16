#pragma once


inline void ThrowIfFailed(const HRESULT hr, const char* msg = "")
{
	if (FAILED(hr))
	{
		throw std::exception(msg);
	}
}