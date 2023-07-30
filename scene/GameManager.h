#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "D3DResourceLeakChecker.h"
#include "IScene.h"
#include <memory>

class GameManager {
public:
	GameManager();
	~GameManager();
	void ChangeScene(IScene* newScene);
	int run();
private:
	D3DResourceLeakChecker leakCheck;
	WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
	ImGuiManager* imguiManager_ = nullptr;
	IScene* nowScene_;
};