#pragma once
#include "DirectXCommon.h"
#include "MathFunction.h"

struct ConstBufferDataDirectionalLight {
	Vector4 color;//ライトの色
	Vector3 direction;//ライトの向き
	float intensity;//輝度
};

class DirectionalLight {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	DirectionalLight();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~DirectionalLight();

	/// <summary>
	/// 作成
	/// </summary>
	void Create();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// グラフィックスコマンドのセット
	/// </summary>
	void SetGraphicsCommand();

private:
	DirectXCommon* dxCommon_ = nullptr;
	//ライティング用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> lightingResource_ = nullptr;
	//書き込み用
	ConstBufferDataDirectionalLight* directionalLightData_ = nullptr;
public:
	//ライトの色
	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };
	//ライトの向き
	Vector3 direction_ = { 0.0f,-1.0f,0.0f };
	//輝度
	float intensity_ = 1.0f;
};