#pragma once
#include "DirectXCommon.h"

struct ConstBufferDataDof {
	bool enable;
	float padding[3];
};

class Dof {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Dof();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Dof();

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
	//Dof用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> dofResource_ = nullptr;
	//書き込み用
	ConstBufferDataDof* dofData_ = nullptr;
public:
	//Dofのフラグ
	bool enable_ = false;
};