#pragma once
#include "DirectXCommon/D3DResourceLeakChecker.h"
#include "WinApp/WinApp.h"
#include "DirectXCommon/DirectXCommon.h"
#include "TextureManager/TextureManager.h"
#include "ImGuiManager/ImGuiManager.h"
#include "Audio/Audio.h"
#include "Input/Input.h"
#include "PostProcess/PostProcess.h"
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
	void run();

private:
	D3DResourceLeakChecker leakCheck;
	WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	ImGuiManager* imguiManager_ = nullptr;
	Audio* audio_ = nullptr;
	Input* input_ = nullptr;
	PostProcess* postProcess_ = nullptr;
	IScene* currentScene_;
};