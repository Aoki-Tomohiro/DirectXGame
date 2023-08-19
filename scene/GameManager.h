#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "D3DResourceLeakChecker.h"
#include "IScene.h"
#include <memory>

class GameManager {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	GameManager();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameManager();

	/// <summary>
	/// シーン切り替え
	/// </summary>
	/// <param name="newScene"></param>
	void ChangeScene(IScene* newScene);

	/// <summary>
	/// ゲームループ
	/// </summary>
	/// <returns></returns>
	int run();

private:
	D3DResourceLeakChecker leakCheck;
	WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	ImGuiManager* imguiManager_ = nullptr;
	IScene* nowScene_;
};