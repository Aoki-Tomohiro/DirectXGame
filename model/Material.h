#pragma once
#include "DirectXCommon.h"
#include "MathFunction.h"

enum LightingMethod {
	LambertianReflectance,
	HalfLambert
};

struct ConstBufferDataMaterial {
	Vector4 color;
	int32_t enableLighting;
	int32_t lightingMethod;
	float padding[2];
	Matrix4x4 uvTransform;
};

class Material {
public:
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
	//uvTransform
	Transform uvTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	//色
	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };
	//ライティングのフラグ
	int32_t enableLighting_ = true;
	//ライティング方式
	int32_t lightingMethod_ = LambertianReflectance;
};