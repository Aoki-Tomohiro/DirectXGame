#pragma once
#include "IScene.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "WorldTransform.h"
#include "ViewProjection.h"
#include "Model.h"
#include "Sprite.h"

class GameScene : public IScene {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	GameScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameScene();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(GameManager* gameManager)override;

	/// <summary>
	/// 更新
	/// </summary>
	void Update(GameManager* gameManager)override;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(GameManager* gameManager)override;

private:
	DirectXCommon* dxCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	ImGuiManager* imguiManager_ = nullptr;
	//画像
	uint32_t textureHandle_ = 0;
	//スプライト
	std::unique_ptr<Sprite> sprite_ = nullptr;
	Vector2 translation_{ 0.0f,0.0f };
	Vector2 scale_{ 1.0f,1.0f };
	float rotation_ = 0.0f;
	Vector4 color_{ 1.0f,1.0f,1.0f,1.0f };
	Vector2 uvTranslation_{ 0.0f,0.0f };
	float uvRotation_ = 0.0f;
	Vector2 uvScale_{ 1.0f,1.0f };
	//モデル
	std::unique_ptr<Model> modelPlane_ = nullptr;
	//ワールドトランスフォーム
	WorldTransform worldTransformPlane_{};
	//ビュープロジェクション
	ViewProjection viewProjection_{};
};