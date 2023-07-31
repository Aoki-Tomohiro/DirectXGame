#pragma once
#include "DirectXCommon.h"

struct ConstBufferDataLensDistortion {
	bool enable;
	float tightness;
	float strength;
};

class LensDistortion {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	LensDistortion();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~LensDistortion();

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
	//LensDistortion用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> LensDistortionResource_ = nullptr;
	//書き込み用
	ConstBufferDataLensDistortion* lensDistortionData_ = nullptr;
public:
	//LensDistortionのフラグ
	bool enable_ = false;
	//歪みのきつさ
	float tightness_ = 2.5f;
	//歪みの強さ
	float strength_ = -0.1f;
};