#include "pch.hpp"

#include "Filesystem.hpp"
#include <codecvt>

namespace zenyth::fs
{
	std::string wideToUtf8(const std::wstring& wstr) {
		if (wstr.empty()) return std::string();

		const int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
		std::string strTo(sizeNeeded, 0);

		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), &strTo[0], sizeNeeded, nullptr, nullptr);
		return strTo;
	}


	std::vector<uint8_t> readFileToByteArray(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);

		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file: " + filename);
		}

		const std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);
		std::vector<uint8_t> buffer(size);

		if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
			throw std::runtime_error("Failed to read file content: " + filename);
		}

		return buffer;
	}

	std::vector<uint8_t> readFileToByteArray(const std::wstring &filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);

		const std::string str = wideToUtf8(filename);

		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file: " + str);
		}

		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);
		std::vector<uint8_t> buffer(size);

		if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
			throw std::runtime_error("Failed to read file content: " + str);
		}

		return buffer;
	}
} // namespace zenyth::fs
