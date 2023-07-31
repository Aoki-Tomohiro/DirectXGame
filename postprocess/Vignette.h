#pragma once
#include "DirectXCommon.h"

struct ConstBufferDataVignette {
	bool enable;
	float intensity;
};

class Vignette {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Vignette();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Vignette();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// グラフィックスコマンドをセット
	/// </summary>
	void SetGraphicsCommand();

private:
	DirectXCommon* dxCommon_ = nullptr;
	//ビネット用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vignetteResource_ = nullptr;
	//書き込み用
	ConstBufferDataVignette* vignetteData_ = nullptr;
public:
	//ビネットのフラグ
	bool enable_ = false;
	//強度
	float intensity_ = 1.0f;
};