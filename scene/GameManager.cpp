#include "GameManager.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "D3DResourceLeakChecker.h"

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
	//シーンの初期化
	nowScene_ = new GameScene();
	nowScene_->Initialize(this);
	//モデルの初期化
	Model::Initialize();
	//スプライトの初期化
	Sprite::Initialize();
}

GameManager::~GameManager() {
	delete nowScene_;
}

void GameManager::ChangeScene(IScene* newScene) {
	nowScene_ = nullptr;
	nowScene_ = newScene;
	nowScene_->Initialize(this);
}

int GameManager::run() {
	while (true) {
		//メインループを抜ける
		if (winApp_->MessageRoop()) {
			break;
		}

		//ImGui受付開始
		imguiManager_->Begin();
		//ゲームシーンの更新
		nowScene_->Update(this);
		//ImGui受付終了
		imguiManager_->End();

		//描画開始
		dxCommon_->PreDraw();
		//ゲームシーンの描画
		nowScene_->Draw(this);
		//ImGuiの描画
		imguiManager_->Draw();
		//描画終了
		dxCommon_->PostDraw();
	}
	//ImGuiの解放処理
	imguiManager_->ShutDown();
	//ゲームウィンドウ削除
	winApp_->CloseGameWindow();
	return 0;
}