#include "ImGuiManager.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

ImGuiManager* ImGuiManager::GetInstance() {
	static ImGuiManager instance;
	return &instance;
}

void ImGuiManager::Initialize() {
	winApp_ = WinApp::GetInstance();
	dxCommon_ = DirectXCommon::GetInstance();
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp_->GetHwnd());
	ImGui_ImplDX12_Init(dxCommon_->GetDevice().Get(), dxCommon_->GetSwapChainDesc().BufferCount,
		dxCommon_->GetRenderTargetViewDesc().Format, dxCommon_->GetSRVDescriptorHeap().Get(), 
		dxCommon_->GetSRVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		dxCommon_->GetSRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
}

void ImGuiManager::Begin() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::End() {
	ImGui::Render();
}

void ImGuiManager::Draw() {
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCommandList().Get());
}

void ImGuiManager::ShutDown() {
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}