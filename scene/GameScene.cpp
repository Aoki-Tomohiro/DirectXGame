#include "GameScene.h"
#include <cassert>

GameScene::GameScene() {};

GameScene::~GameScene() {};

void GameScene::Initialize(GameManager* gameManager) {
	dxCommon_ = DirectXCommon::GetInstance();
	textureManager_ = TextureManager::GetInstance();
	imguiManager_ = ImGuiManager::GetInstance();
	//画像読み込み
	textureHandle_ = textureManager_->Load("resources/uvChecker.png");
	//スプライトの作成
	sprite_ = std::make_unique<Sprite>();
	sprite_->Create(textureHandle_, translation_);
	//モデルの作成
	modelPlane_ = std::make_unique<Model>();
	modelPlane_->CreateFromOBJ("resources", "plane.obj");
};

void GameScene::Update(GameManager* gameManager) {
	//スプライト
	sprite_->SetTranslation(translation_);
	sprite_->SetScale(scale_);
	sprite_->SetRotation(rotation_);
	sprite_->SetColor(color_);
	sprite_->SetUVTranslation(uvTranslation_);
	sprite_->SetUVRotation(uvRotation_);
	sprite_->SetUVScale(uvScale_);
	//モデル
	worldTransformPlane_.UpdateMatrix();
	//ビュープロジェクションの更新
	viewProjection_.UpdateMatrix();

	ImGui::Begin(" ");
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
	if (ImGui::TreeNode("Plane")) {
		ImGui::DragFloat3("translate", &worldTransformPlane_.translation_.x, 0.01f);
		ImGui::DragFloat3("rotate", &worldTransformPlane_.rotation_.x, 0.01f);
		ImGui::DragFloat3("scale", &worldTransformPlane_.scale_.x, 0.01f);
		ImGui::TreePop();
	}
	ImGui::End();
};

void GameScene::Draw(GameManager* gameManager) {
	//モデルの描画
	modelPlane_->Draw(worldTransformPlane_, viewProjection_);
	//スプライトの描画
	sprite_->Draw();
};