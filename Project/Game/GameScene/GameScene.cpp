#include "GameScene.h"
#include <cassert>

GameScene::GameScene() {};

GameScene::~GameScene() {};

void GameScene::Initialize(GameManager* gameManager) {
	//TextureManagerのインスタンスを取得
	textureManager_ = TextureManager::GetInstance();
	//Audioのインスタンスを取得
	audio_ = Audio::GetInstance();
	//Inputのインスタンスを取得
	input_ = Input::GetInstance();
	//PostProcessのインスタンスを取得
	postProcess_ = PostProcess::GetInstance();
	//衝突マネージャーの作成
	collisionManager_ = new CollisionManager();
	//デバッグカメラの生成
	debugCamera_ = new DebugCamera();
};

void GameScene::Update(GameManager* gameManager) {
	XINPUT_STATE joyState{};
	if (input_->GetJoystickState(joyState)) {
		if (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
			isDebugCameraActive_ = true;
		}
		else if(joyState.Gamepad.wButtons & XINPUT_GAMEPAD_B) {
			isDebugCameraActive_ = false;
		}
	}

	//デバッグカメラの更新
	debugCamera_->Update();
	//デバッグカメラの切り替え
	if (input_->IsPushKeyEnter(DIK_1)) {
		if (isDebugCameraActive_ == false) {
			isDebugCameraActive_ = true;
		}
		else {
			isDebugCameraActive_ = false;
		}
	}
	//ビュープロジェクションの更新
	if (isDebugCameraActive_ == true) {
		viewProjection_.matView_ = debugCamera_->GetViewProjection().matView_;
		viewProjection_.matProjection_ = debugCamera_->GetViewProjection().matProjection_;
	}
	else {
		viewProjection_.UpdateMatrix();
	}

	ImGui::Begin(" ");
	//ポストプロセス
	ImGui::Checkbox("PostProcess", &postProcess_->isActive);
	ImGui::Checkbox("Bloom", &postProcess_->isBloomActive);
	ImGui::Checkbox("Fog", &postProcess_->isFogActive);
	ImGui::Checkbox("DoF", &postProcess_->isDofActive);
	ImGui::Checkbox("LensDistortion", &postProcess_->isLensDistortionActive);
	ImGui::Checkbox("Vignette", &postProcess_->isVignetteActive);
	//デバッグカメラ
	ImGui::Checkbox("DebugCamera", &isDebugCameraActive_);
	ImGui::Text("1 : DebugCamera");
	ImGui::Text("WASD : Move up/down/left/right");
	ImGui::Text("MouseWheel : Move forward/backward");
	ImGui::Text("Left Right : RotateX");
	ImGui::Text("UP DOWN : RotateY");
	ImGui::End();
};

void GameScene::Draw(GameManager* gameManager) {

#pragma region ポストプロセス
	//ポストプロセスの描画前処理
	postProcess_->PreDraw();
	//ポストプロセスの描画後処理
	postProcess_->PostDraw();
#pragma endregion

};