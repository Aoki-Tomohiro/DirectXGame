#pragma once
#include "DirectXCommon.h"

struct ConstBufferDataFog {
	bool enable;
	float scale;
	float attenuationRate;
};

class Fog {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Fog();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Fog();

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
	//フォグ用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> fogResource_ = nullptr;
	//書き込み用
	ConstBufferDataFog* fogData_ = nullptr;
public:
	//フォグのフラグ
	bool enable_ = false;
	//スケール
	float scale_ = 0.5f;
	//減衰率
	float attenuationRate_ = 2.0f;
};