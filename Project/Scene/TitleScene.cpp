#include "TitleScene.h"
#include "GameScene.h"
#include "GameManager.h"

TitleScene::TitleScene() {};

TitleScene::~TitleScene() {};

void TitleScene::Initialize(GameManager* gameManager) {
	dxCommon_ = DirectXCommon::GetInstance();
	imguiManager_ = ImGuiManager::GetInstance();
}

void TitleScene::Update(GameManager* gameManager) {}

void TitleScene::Draw(GameManager* gameManager) {}