#pragma once
#include "DirectXCommon.h"
#include "MathFunction.h"

class Material {
public:
	//ライティングの種類
	enum class LightingMethod {
		LambertianReflectance,
		HalfLambert
	};

	//定数バッファ用の構造体
	struct ConstBufferDataMaterial {
		Vector4 color;
		int32_t enableLighting;
		LightingMethod lightingMethod;
		float padding[2];
		Matrix4x4 uvTransform;
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	Material();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Material();

	/// <summary>
	/// マテリアルの作成
	/// </summary>
	void Create();

	/// <summary>
	/// マテリアルの更新
	/// </summary>
	void Update();

	/// <summary>
	/// グラフィックスコマンドのセット
	/// </summary>
	void SetGraphicsCommand();

private:
	DirectXCommon* dxCommon_ = nullptr;
	//マテリアル用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	//書き込み用
	ConstBufferDataMaterial* materialData_ = nullptr;
public:
	//uv座標
	Vector2 translation_{ 0.0f,0.0f };
	//uv角度
	float rotation_ = 0.0f;
	//uvスケール
	Vector2 scale_{ 1.0f,1.0f };
	//色
	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };
	//ライティングのフラグ
	int32_t enableLighting_ = true;
	//ライティング方式
	LightingMethod lightingMethod_ = LightingMethod::HalfLambert;
};