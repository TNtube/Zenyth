#pragma once

namespace zenyth::fs
{
	std::vector<uint8_t> readFileToByteArray(const std::string& filename);
	std::vector<uint8_t> readFileToByteArray(const std::wstring& filename);
} // namespace zenyth::fs
