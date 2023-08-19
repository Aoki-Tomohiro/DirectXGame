#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "imgui.h"

class ImGuiManager {
public:
	static ImGuiManager* GetInstance();
	void Initialize();
	void Begin();
	void End();
	void Draw();
	void ShutDown();
private:
	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	const ImGuiManager& operator = (const ImGuiManager&) = delete;
private:
	WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
};