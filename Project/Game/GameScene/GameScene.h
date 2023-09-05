#pragma once
#include "GameManager/IScene.h"
#include "TextureManager/TextureManager.h"
#include "Audio/Audio.h"
#include "Input/Input.h"
#include "PostProcess/PostProcess.h"
#include "DebugCamera/DebugCamera.h"
#include "ImGuiManager/ImGuiManager.h"
#include "TransformationMatrix/WorldTransform.h"
#include "TransformationMatrix/ViewProjection.h"
#include "Model/Model.h"
#include "Sprite/Sprite.h"
#include "CollisionManager/CollisionManager.h"

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
	//DirectX
	DirectXCommon* dxCommon_ = nullptr;
	//TextureManager
	TextureManager* textureManager_ = nullptr;
	//Audio
	Audio* audio_ = nullptr;
	//Input
	Input* input_ = nullptr;
	//PostProcess
	PostProcess* postProcess_ = nullptr;
	//DebugCamera
	DebugCamera* debugCamera_ = nullptr;
	//DebugCameraの切り替え
	bool isDebugCameraActive_ = false;
	//画像
	uint32_t textureHandle_ = 0;
	//音声データ
	uint32_t audioHandle_ = 0;
	uint32_t audioHandle2_ = 0;
	//スプライトの調整項目
	Sprite* sprite_ = nullptr;
	Sprite* sprite2_ = nullptr;
	Vector2 translation_{ 0.0f,0.0f };
	Vector2 scale_{ 1.0f,1.0f };
	float rotation_ = 0.0f;
	Vector4 color_{ 1.0f,1.0f,1.0f,1.0f };
	Vector2 uvTranslation_{ 0.0f,0.0f };
	float uvRotation_ = 0.0f;
	Vector2 uvScale_{ 1.0f,1.0f };
	//モデル
	Model* modelPlane_ = nullptr;
	//モデルのワールドトランスフォーム
	WorldTransform worldTransformPlane_{};
	//ビュープロジェクション
	ViewProjection viewProjection_{};
	//衝突マネージャー
	CollisionManager* collisionManager_ = nullptr;
};