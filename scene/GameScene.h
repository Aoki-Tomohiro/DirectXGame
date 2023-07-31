#pragma once
#include "IScene.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "Model.h"
#include "Texture.h"
#include "TransformationMatrix.h"

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
	ImGuiManager* imguiManager_ = nullptr;
	//テクスチャ
	std::unique_ptr<Texture> texture_ = nullptr;
	//三角形
	std::vector<VertexData> triangle_;
	std::unique_ptr<Model> model_ = nullptr;
	std::unique_ptr<TransformationMatrix> transformationMatrix_ = nullptr;
	//球
	std::vector<VertexData> sphere_;
	std::unique_ptr<Model> modelSphere_ = nullptr;
	std::unique_ptr<TransformationMatrix> transformationMatrixSphere_ = nullptr;
	//スプライト
	std::vector<VertexData> sprite_;
	std::unique_ptr<Model> modelSprite_ = nullptr;
	std::unique_ptr<TransformationMatrix> transformationMatrixSprite_ = nullptr;
	//utahTeapot
	std::unique_ptr<Model> modelUtahTeapot_ = nullptr;
	std::unique_ptr<TransformationMatrix> transformationMatrixUtahTeapot_ = nullptr;
	//StanfordBunny
	std::unique_ptr<Model> modelStanfordBunny_ = nullptr;
	std::unique_ptr<TransformationMatrix> transformationMatrixStanfordBunny_ = nullptr;
	//imgui用
	bool drawTriangle_ = true;
	bool drawSphere_ = false;
	bool drawSprite_ = false;
	bool drawUtahTeapot_ = false;
	bool drawStanfordBunny_ = false;
};