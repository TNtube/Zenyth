#pragma once
#include "Application.hpp"

class SandboxApp : public Zenyth::Application {
public:
	explicit SandboxApp() : Application({ .title = L"Sandbox", .width = 1280, .height = 720 }) {}

protected:
	void OnInit() override;
	void OnUpdate(float dt) override;
	void OnRender() override;

	void OnEvent(const Zenyth::Event& e) override;

	void OnShutdown() override;
};
