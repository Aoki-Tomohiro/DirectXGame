#pragma once
#include "DirectXCommon.h"
#include <array>

class TextureManager {
public:
	struct Texture {
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandleSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandleSRV;
	};

	static const size_t kNumDescriptors = 256;

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static TextureManager* GetInstance();

	/// <summary>
	/// シングルトンインスタンスの削除
	/// </summary>
	static void DeleteInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// テクスチャ読み込み
	/// </summary>
	uint32_t Load(const std::string& filePath);

	/// <summary>
	/// マルチパス用のテクスチャの作成
	/// </summary>
	uint32_t CreateMultiPassTexture(DXGI_FORMAT format, const float clearColor[]);

	/// <summary>
	/// テクスチャリソースの作成
	/// </summary>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	/// <summary>
	/// マルチパス用リソースの作成 
	/// </summary>
	/// <param name="device"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateMultiPassResource(int32_t width, int32_t height, DXGI_FORMAT format, const float clearColor[]);

	/// <summary>
	/// テクスチャをアップロード
	/// </summary>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);

	/// <summary>
    /// シェーダーリソースビューの作成
    /// </summary>
    /// <param name="resource"></param>
    /// <param name="metadata"></param>
    /// <param name="index"></param>
    void CreateShaderResourceView(const Microsoft::WRL::ComPtr<ID3D12Resource>& resource, const DirectX::TexMetadata& metadata);

	/// <summary>
	/// マルチパス用のシェーダーリソースビューの作成
	/// </summary>
	void CreateMultiPassShaderResourceView(const Microsoft::WRL::ComPtr<ID3D12Resource>& resource, DXGI_FORMAT format);

	/// <summary>
	/// ディスクリプタヒープをセット
	/// </summary>
	void SetGraphicsDescriptorHeap();

	/// <summary>
	/// ディスクリプタテーブルをセット
	/// </summary>
	/// <param name="textureHandle"></param>
	void SetGraphicsRootDescriptorTable(UINT rootParameterIndex, uint32_t textureHandle);

	/// <summary>
	/// リソース情報を取得
	/// </summary>
	/// <param name="textureHandle"></param>
	/// <returns></returns>
	D3D12_RESOURCE_DESC GetResourceDesc(uint32_t textureHandle);

	/// <summary>
	/// リソースを取得
	/// </summary>
	ID3D12Resource* GetTextureResource(uint32_t textureHandle) { return textures_[textureHandle].resource.Get(); };

private:
	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;
private:
	static TextureManager* instance;
	DirectXCommon* dxCommon_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_ = nullptr;
	std::array<Texture, kNumDescriptors> textures_;
	uint32_t srvIndex_ = -1;
};