#pragma once
#include "WinApp.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dxgidebug.h>
#include <vector>
#include "DirectXTex.h"
#include "d3dx12.h"
#include <wrl.h>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")

class DirectXCommon {
public:
	//インクリメントサイズ
	static uint32_t descriptorSizeSRV;
	static uint32_t descriptorSizeRTV;
	static uint32_t descriptorSizeDSV;

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static DirectXCommon* GetInstance();

	static void DeleteInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// DXGIデバイスの生成
	/// </summary>
	void CreateDXGIDevice();

	/// <summary>
	/// コマンドの生成
	/// </summary>
	void CreateCommand();

	/// <summary>
	/// スワップチェーンの壊死性
	/// </summary>
	void CreateSwapChain();

	/// <summary>
	/// フェンスの生成
	/// </summary>
	void CreateFence();

	/// <summary>
	/// ディスクリプタヒープの生成
	/// </summary>
	/// <param name="device"></param>
	/// <param name="heapType"></param>
	/// <param name="numDescriptors"></param>
	/// <param name="shaderVisible"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(const Microsoft::WRL::ComPtr<ID3D12Device>& device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	
	/// <summary>
	///マルチパス用リソースの作成 
	/// </summary>
	/// <param name="device"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateMultiPassResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device);

	/// <summary>
	/// 縮小バッファの作成
	/// </summary>
	/// <param name="device"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateShrinkBuffer(const Microsoft::WRL::ComPtr<ID3D12Device>& device);

	/// <summary>
	/// バッファリソースの作成
	/// </summary>
	/// <param name="device"></param>
	/// <param name="sizeInBytes"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes);
	
	/// <summary>
	/// 深度バッファの作成
	/// </summary>
	/// <param name="device"></param>
	/// <param name="width"></param>
	/// <param name="height"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height);
	
	/// <summary>
	/// レンダーターゲットビューの作成
	/// </summary>
	void CreateRenderTargetView();
	
	/// <summary>
	/// シェーダーリソースビューの作成
	/// </summary>
	/// <param name="resource"></param>
	/// <param name="metadata"></param>
	/// <param name="index"></param>
	uint32_t CreateShaderResourceView(const Microsoft::WRL::ComPtr<ID3D12Resource>& resource, const DirectX::TexMetadata& metadata);
	
	/// <summary>
	/// 深度ステンシルビューの作成
	/// </summary>
	void CreateDepthStencilView();

	/// <summary>
	/// マルチパス用レンダーターゲットビューの作成
	/// </summary>
	/// <param name="resource"></param>
	/// <returns></returns>
	void CreateMultiPassRTV(const Microsoft::WRL::ComPtr<ID3D12Resource>& resource);

	/// <summary>
	/// マルチパス用シェーダーリソースビューの作成
	/// </summary>
	/// <param name="resource"></param>
	/// <returns></returns>
	void CreateMultiPassSRV(const Microsoft::WRL::ComPtr<ID3D12Resource>& resource, DXGI_FORMAT format);

	/// <summary>
	/// CPUディスクリプタハンドルを取得
	/// </summary>
	/// <param name="descriptorHeap"></param>
	/// <param name="descriptorSize"></param>
	/// <param name="index"></param>
	/// <returns></returns>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, const uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// GPUディスクリプタハンドルを取得
	/// </summary>
	/// <param name="descriptorHeap"></param>
	/// <param name="descriptorSize"></param>
	/// <param name="index"></param>
	/// <returns></returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, const uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// 一パス目の描画前処理
	/// </summary>
	void FirstPassPreDraw();

	/// <summary>
	/// 一パス目の描画後処理
	/// </summary>
	void FirstPassPostDraw();

	/// <summary>
	/// 二パス目の描画前処理
	/// </summary>
	void SecondPassPreDraw();

	/// <summary>
	/// 二パス目の描画後処理
	/// </summary>
	void SecondPassPostDraw();

	/// <summary>
	/// 横ぼかし描画前処理
	/// </summary>
	void HorizontalBlurPreDraw();

	/// <summary>
	/// 横ぼかし描画後処理
	/// </summary>
	void HorizontalBlurPostDraw();

	/// <summary>
	/// 縦ぼかし描画前処理
	/// </summary>
	void VerticalBlurPreDraw();

	/// <summary>
	/// 縦ぼかし描画後処理
	/// </summary>
	void VerticalBlurPostDraw();

	/// <summary>
	/// 横縮小ぼかし描画前処理
	/// </summary>
	void HorizontalShrinkBlurPreDraw();

	/// <summary>
	/// 横縮小ぼかし描画後処理
	/// </summary>
	void HorizontalShrinkBlurPostDraw();

	/// <summary>
	/// 縦縮小ぼかし描画前処理
	/// </summary>
	void VerticalShrinkBlurPreDraw();

	/// <summary>
	/// 縦縮小ぼかし描画後処理
	/// </summary>
	void VerticalShrinkBlurPostDraw();

	/// <summary>
	/// ウィンドウズアプリケーションの取得
	/// </summary>
	/// <returns></returns>
	WinApp* GetWinApp() { return winApp_; };

	/// <summary>
	/// デバイスの取得
	/// </summary>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() { return device_; };

	/// <summary>
	/// スワップチェーンデスクの取得
	/// </summary>
	/// <returns></returns>
	DXGI_SWAP_CHAIN_DESC1 GetSwapChainDesc() { return swapChainDesc_; };

	/// <summary>
	/// レンダーターゲットビューデスクの取得
	/// </summary>
	/// <returns></returns>
	D3D12_RENDER_TARGET_VIEW_DESC GetRenderTargetViewDesc() { return rtvDesc_; };

	/// <summary>
	/// ディスクリプタヒープの取得
	/// </summary>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSRVDescriptorHeap() { return srvDescriptorHeap_; };

	/// <summary>
	/// コマンドリストの取得
	/// </summary>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return commandList_; };

private:
	DirectXCommon() = default;
	~DirectXCommon() = default;
	DirectXCommon(const DirectXCommon&) = delete;
	const DirectXCommon& operator=(const DirectXCommon&) = delete;
private:
	//インスタンス
	static DirectXCommon* instance;
	//WinApp
	WinApp* winApp_ = nullptr;
	//デバッグレイヤー
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
	//DXGIファクトリー
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
	//アダプタ
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;
	//デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;
	//コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;
	//コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
	//コマンドリストを生成
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
	//スワップチェーン
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
	//ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_ = nullptr;
	//リソース
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2] = { nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;
	//フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
	uint64_t fenceValue_ = 0;
	//viewの場所
	uint32_t rtvIndex_ = -1;
	uint32_t srvIndex_ = 0;
	//マルチパスレンダリング用RTVDescriptorHeap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> multiPassRTVDescriptorHeap_ = nullptr;
	//一パス目のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> firstPassResource_ = nullptr;
	//Blur用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> blurResource_[2] = { nullptr };
	//縮小バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> shrinkBlurResource_[2] = { nullptr };
	//深度書き込み用
	Microsoft::WRL::ComPtr<ID3D12Resource> linearDepthStensilResource_ = nullptr;
};