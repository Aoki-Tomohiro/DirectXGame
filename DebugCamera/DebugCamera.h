#pragma once
#include "Input.h"
#include "WorldTransform.h"
#include "ViewProjection.h"
#include "MathFunction.h"

class DebugCamera{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	DebugCamera();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~DebugCamera();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// ビュープロジェクションを取得
	/// </summary>
	/// <returns></returns>
	const ViewProjection& GetViewProjection() { return viewProjection_; };

	/// <summary>
	/// 追従対象の設定
	/// </summary>
	/// <param name="target"></param>
	void SetTarget(const WorldTransform* target) { target_ = target; };

private:
	//入力クラス
	Input* input_ = nullptr;
	//ワールドトランスフォーム
	WorldTransform worldTransform_{};
	//ビュープロジェクション
	ViewProjection viewProjection_{};
	//オフセット
	Vector3 offset_ = { 0.0f,0.0f,-50.0f };
	//累積回転行列
	Matrix4x4 matRot_ = MakeIdentity4x4();
	//追従対象
	const WorldTransform* target_ = nullptr;
};

