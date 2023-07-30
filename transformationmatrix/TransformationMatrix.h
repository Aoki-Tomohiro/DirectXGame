#pragma once
#include "DirectXCommon.h"
#include "MathFunction.h"

struct ConstBufferDataTransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

class TransformationMatrix {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	TransformationMatrix();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~TransformationMatrix();

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
	void SetGraphicsCommand();

private:
	DirectXCommon* dxCommon_ = nullptr;
	//WVPResource
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr;
	//書き込み用
	ConstBufferDataTransformationMatrix* wvpData_ = nullptr;
public:
	//カメラのトランスフォーム
	Transform cameraTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
	//トランスフォーム
	Transform transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	//ワールド行列
	Matrix4x4 worldMatrix_{};
	//カメラのワールド行列
	Matrix4x4 cameraMatrix_{};
	//ビュー行列
	Matrix4x4 viewMatrix_{};
	//プロジェクション行列
	Matrix4x4 projectionMatrix_{};
	//ワールドビュープロジェクション行列
	Matrix4x4 worldViewProjectionMatrix_{};
};