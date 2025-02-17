#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

// Windows
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <directx/d3dx12.h>
#include <DirectXTex.h>


// STL
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <functional>
#include <optional>
#include <filesystem>
#include <format>
#include <exception>
#include <concepts>
#include <type_traits>

#include <wrl.h>
#include <shellapi.h>
#include <cmath>
#include <cstdint>
