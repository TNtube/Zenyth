#pragma once

#define DELETE_COPY_CTOR(ClassName) \
	ClassName(const ClassName&) = delete; \
	ClassName& operator=(const ClassName&) = delete;

#define DELETE_MOVE_CTOR(ClassName) \
	ClassName(ClassName&&) = delete; \
	ClassName& operator=(ClassName&&) = delete;

#define DEFAULT_COPY_CTOR(ClassName) \
	ClassName(const ClassName&) = default; \
	ClassName& operator=(const ClassName&) = default;

#define DEFAULT_MOVE_CTOR(ClassName) \
	ClassName(ClassName&&) noexcept = default; \
	ClassName& operator=(ClassName&&) noexcept = default;


inline void ThrowIfFailed(const HRESULT hr, const char* msg = "")
{
	if (FAILED(hr))
	{
		throw std::exception(msg);
	}
}