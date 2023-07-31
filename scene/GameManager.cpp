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
	//ImGuiの初期化
	imguiManager_ = ImGuiManager::GetInstance();
	imguiManager_->Initialize();
	//ガウシアンブラーの初期化
	gaussianBlur_ = new GaussianBlur();
	gaussianBlur_->Initialize();
	//PostProcessの初期化
	postProcess_ = new PostProcess();
	postProcess_->Initialize();
	//シーンの初期化
	nowScene_ = new GameScene();
	nowScene_->Initialize(this);
	//モデルの初期化
	Model::Initialize();
}

GameManager::~GameManager() {
	delete nowScene_;
	//DirectXCommonのインスタンスを削除
	dxCommon_->DeleteInstance();
	//WinAppのインスタンスを削除
	winApp_->DeleteInstance();
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
		//ポストプロセスの更新
		postProcess_->Update();
		ImGui::Begin(" ");
		if (ImGui::TreeNode("PostProcess")) {
			ImGui::Checkbox("Fog:enable", &postProcess_->GetFog()->enable_);
			ImGui::DragFloat("Fog:scale", &postProcess_->GetFog()->scale_,0.01f);
			ImGui::DragFloat("Fog:attenuationRate", &postProcess_->GetFog()->attenuationRate_, 0.01f);
			ImGui::Checkbox("Dof:enable", &postProcess_->GetDof()->enable_);
			ImGui::Checkbox("LensDistortion:enable", &postProcess_->GetLensDistortion()->enable_);
			ImGui::Checkbox("Vignette:enable", &postProcess_->GetVignette()->enable_);
			ImGui::TreePop();
		}
		ImGui::End();
		//ImGui受付終了
		imguiManager_->End();

		//一パス目描画開始
		dxCommon_->FirstPassPreDraw();
		//ゲームシーンの描画
		nowScene_->Draw(this);
		//一パス目描画終了
		dxCommon_->FirstPassPostDraw();

		//横ぼかし
		dxCommon_->HorizontalBlurPreDraw();
		gaussianBlur_->HorizontalBlur();
		dxCommon_->HorizontalBlurPostDraw();

		//縦ぼかし
		dxCommon_->VerticalBlurPreDraw();
		gaussianBlur_->VerticalBlur();
		dxCommon_->VerticalBlurPostDraw();

		//横縮小ぼかし
		dxCommon_->HorizontalShrinkBlurPreDraw();
		gaussianBlur_->HorizontalShrinkBlur();
		dxCommon_->HorizontalShrinkBlurPostDraw();

		//縦縮小ぼかし
		dxCommon_->VerticalShrinkBlurPreDraw();
		gaussianBlur_->VerticalShrinkBlur();
		dxCommon_->VerticalShrinkBlurPostDraw();

		//二パス目描画開始
		dxCommon_->SecondPassPreDraw();
		//ポストプロセス
		postProcess_->Draw();
		//ImGuiの描画
		imguiManager_->Draw();
		//二パス目描画終了
		dxCommon_->SecondPassPostDraw();
	}
	//ImGuiの解放処理
	imguiManager_->ShutDown();
	//ゲームウィンドウ削除
	winApp_->CloseGameWindow();
	return 0;
}