#pragma once
#include "DirectXCommon/DirectXCommon.h"
#include "Math/MathFunction.h"

class DirectionalLight {
public:
	//定数バッファ用の構造体
	struct ConstBufferDataDirectionalLight {
		Vector4 color;//ライトの色
		Vector3 direction;//ライトの向き
		float intensity;//輝度
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	DirectionalLight();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~DirectionalLight();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// グラフィックスコマンドのセット
	/// </summary>
	void SetGraphicsCommand(UINT rootParameterIndex);

private:
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