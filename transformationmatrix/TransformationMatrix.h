#pragma once
#include "DirectXCommon.h"
#include "WorldTransform.h"
#include "ViewProjection.h"
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
	/// マッピングする
	/// </summary>
	void Update(const WorldTransform& worldTransform, const ViewProjection& viewProjection);

	/// <summary>
	/// グラフィックスコマンドのセット
	/// </summary>
	void SetGraphicsCommand(UINT rootParameterIndex);

private:
	//WVPResource
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr;
	//書き込み用
	ConstBufferDataTransformationMatrix* wvpData_ = nullptr;
};