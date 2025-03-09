#pragma once
#include "CommandManager.hpp"

namespace Zenyth::Renderer
{
	void Initialize(bool useWrapDevice = false);
	void Shutdown();

	extern Microsoft::WRL::ComPtr<ID3D12Device> pDevice;
	extern std::unique_ptr<CommandManager> pCommandManager;
}
