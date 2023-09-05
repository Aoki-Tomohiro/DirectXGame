#include "GameScene.h"
#include <cassert>

GameScene::GameScene() {};

GameScene::~GameScene() {
	delete collisionManager_;
	delete debugCamera_;
	delete sprite_;
	delete sprite2_;
	delete modelPlane_;
};

void GameScene::Initialize(GameManager* gameManager) {
	//DirectXのインスタンスを取得
	dxCommon_ = DirectXCommon::GetInstance();
	//TextureManagerのインスタンスを取得
	textureManager_ = TextureManager::GetInstance();
	//Audioのインスタンスを取得
	audio_ = Audio::GetInstance();
	//Inputのインスタンスを取得
	input_ = Input::GetInstance();
	//PostProcessのインスタンスを取得
	postProcess_ = PostProcess::GetInstance();
	//デバッグカメラの生成
	debugCamera_ = new DebugCamera();
	//追従対象の設定
	debugCamera_->SetTarget(&worldTransformPlane_);
	//画像読み込み
	textureHandle_ = textureManager_->Load("Project/Resources/uvChecker.png");
	//音声データ読み込み
	audioHandle_ = audio_->SoundLoadWave("Project/Resources/Alarm01.wav");
	audioHandle2_ = audio_->SoundLoadWave("Project/Resources/Alarm02.wav");
	//スプライトの作成
	sprite_ = new Sprite();
	sprite_->Create(textureHandle_, translation_);
	sprite2_ = new Sprite();
	sprite2_->Create(textureHandle_, translation_);
	//モデルの作成
	modelPlane_ = new Model();
	modelPlane_->CreateFromOBJ("Project/Resources", "plane.obj");
	//衝突マネージャーの作成
	collisionManager_ = new CollisionManager();
	//再生
	audio_->SoundPlayWave(audioHandle_, false);
	//audio_->SoundPlayWave(audioHandle2_, true);
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

	//スプライト
	sprite_->SetTranslation(translation_);
	sprite_->SetScale(scale_);
	sprite_->SetRotation(rotation_);
	sprite_->SetColor(color_);
	sprite_->SetUVTranslation(uvTranslation_);
	sprite_->SetUVRotation(uvRotation_);
	sprite_->SetUVScale(uvScale_);
	//モデルのワールドトランスフォームの更新
	worldTransformPlane_.UpdateMatrix();
	//デバッグカメラの更新
	debugCamera_->Update();
	//デバッグカメラの切り替え
	if (input_->IsPushKeyEnter(DIK_1)) {
		if (isDebugCameraActive_ == false) {
			isDebugCameraActive_ = true;
			audio_->SoundPlayWave(audioHandle_, false);
		}
		else {
			isDebugCameraActive_ = false;
			audio_->SoundPlayWave(audioHandle_, false);
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
	//スプライト
	if (ImGui::TreeNode("Sprite")) {
		ImGui::DragFloat2("translation", &translation_.x, 1.0f, 0.0f, 1280.0f);
		ImGui::DragFloat2("scale", &scale_.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("rotation", &rotation_);
		ImGui::ColorEdit4("color", &color_.x);
		ImGui::DragFloat2("UVTranslate", &uvTranslation_.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UVScale", &uvScale_.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UVRotate", &uvRotation_);
		ImGui::TreePop();
	}
	//モデル
	if (ImGui::TreeNode("Plane")) {
		ImGui::DragFloat3("translate", &worldTransformPlane_.translation_.x, 1.0f);
		ImGui::DragFloat3("rotate", &worldTransformPlane_.rotation_.x, 0.01f);
		ImGui::DragFloat3("scale", &worldTransformPlane_.scale_.x, 0.01f);
		ImGui::TreePop();
	}
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
	//スプライトの描画
	sprite_->Draw();
	sprite2_->Draw();

#pragma region ポストプロセス
	//ポストプロセスの描画前処理
	postProcess_->PreDraw();
	//モデルの描画
	modelPlane_->Draw(worldTransformPlane_, viewProjection_);
	//ポストプロセスの描画後処理
	postProcess_->PostDraw();
#pragma endregion

};