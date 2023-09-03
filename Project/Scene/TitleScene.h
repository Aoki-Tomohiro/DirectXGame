#pragma once
#include "IScene.h"
#include "DirectXCommon/DirectXCommon.h"
#include "ImGuiManager/ImGuiManager.h"

class TitleScene : public IScene{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	TitleScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~TitleScene();

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
};