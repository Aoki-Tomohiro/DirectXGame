#pragma once
#include "DirectXCommon.h"

class Texture {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Texture();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Texture();

	/// <summary>
	/// テクスチャ読み込み
	/// </summary>
	uint32_t Load(const std::string& filePath);

	/// <summary>
	/// テクスチャリソースの作成
	/// </summary>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	/// <summary>
	/// テクスチャをアップロード
	/// </summary>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);

	/// <summary>
	/// グラフィックスコマンドのセット
	/// </summary>
	void SetGraphicsCommand();

	/// <summary>
	/// テクスチャハンドルを取得
	/// </summary>
	/// <returns></returns>
	uint32_t GetSRVIndex() { return srvIndex_; };

private:
	DirectXCommon* dxCommon_ = nullptr;
	//テクスチャ用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_ = nullptr;
	//中間リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource_ = nullptr;
	//srvの場所
	uint32_t srvIndex_ = 0;
};