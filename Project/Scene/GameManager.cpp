#include "GameManager.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "GlobalVariables/GlobalVariables.h"

GameManager::GameManager() {
	//ゲームウィンドウ作成
	winApp_ = WinApp::GetInstance();
	winApp_->CreateGameWindow(L"CG2WindowClass", winApp_->kClientWidth, winApp_->kClientHeight);
	//DirectXの初期化
	dxCommon_ = DirectXCommon::GetInstance();
	dxCommon_->Initialize();
	//TextureManagerの初期化
	textureManager_ = TextureManager::GetInstance();
	textureManager_->Initialize();
	//ImGuiの初期化
	imguiManager_ = ImGuiManager::GetInstance();
	imguiManager_->Initialize();
	//Audioの初期化
	audio_ = Audio::GetInstance();
	audio_->Initialize();
	//Inputの初期化
	input_ = Input::GetInstance();
	input_->Initialize();
	//ポストプロセスの初期化
	postProcess_ = PostProcess::GetInstance();
	postProcess_->Initialize();
	//グローバル変数の読み込み
	GlobalVariables::GetInstance()->LoadFiles();
	//シーンの初期化
	currentScene_ = new GameScene();
	currentScene_->Initialize(this);
}

GameManager::~GameManager() {
	delete currentScene_;
	currentScene_ = nullptr;
	PostProcess::DeleteInstance();
	Input::DeleteInstance();
	Audio::DeleteInstance();
	ImGuiManager::DeleteInstance();
	TextureManager::DeleteInstance();
	DirectXCommon::DeleteInstance();
	WinApp::DeleteInstance();
}

void GameManager::ChangeScene(IScene* newScene) {
	currentScene_ = nullptr;
	currentScene_ = newScene;
	currentScene_->Initialize(this);
}

void GameManager::run() {
	while (true) {
		//メインループを抜ける
		if (winApp_->MessageRoop()) {
			break;
		}

		//ImGui受付開始
		imguiManager_->Begin();
		//Inputの更新
		input_->Update();
		//グローバル変数の更新
		GlobalVariables::GetInstance()->Update();
		//ゲームシーンの更新
		currentScene_->Update(this);
		//ポストプロセスの更新
		postProcess_->Update();
		//ImGui受付終了
		imguiManager_->End();

		//描画開始
		dxCommon_->PreDraw();
		//ゲームシーンの描画
		currentScene_->Draw(this);
		//ポストプロセスの描画
		postProcess_->Draw();
		//ImGuiの描画
		imguiManager_->Draw();
		//描画終了
		dxCommon_->PostDraw();
	}
	//ImGuiの解放処理
	imguiManager_->ShutDown();
	//Audioの解放処理
	audio_->Finalize();
	//ゲームウィンドウ削除
	winApp_->CloseGameWindow();
}