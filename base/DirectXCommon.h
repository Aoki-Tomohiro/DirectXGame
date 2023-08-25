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
	static uint32_t descriptorSizeRTV;
	static uint32_t descriptorSizeDSV;

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static DirectXCommon* GetInstance();

	/// <summary>
	/// シングルトンインスタンスの削除
	/// </summary>
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
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	/// <summary>
	/// バッファリソースの作成
	/// </summary>
	/// <param name="device"></param>
	/// <param name="sizeInBytes"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
	
	/// <summary>
	/// 深度バッファの作成
	/// </summary>
	/// <param name="device"></param>
	/// <param name="width"></param>
	/// <param name="height"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(int32_t width, int32_t height);
	
	/// <summary>
	/// レンダーターゲットビューの作成
	/// </summary>
	void CreateRenderTargetView();
	
	/// <summary>
	/// 深度ステンシルビューの作成
	/// </summary>
	void CreateDepthStencilView(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, const Microsoft::WRL::ComPtr<ID3D12Resource>& resource);

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
	/// バックバッファをセット
	/// </summary>
	void SetBackBuffer();

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画後処理
	/// </summary>
	void PostDraw();

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
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_ = nullptr;
	//リソース
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2] = { nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;
	//フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
	uint64_t fenceValue_ = 0;
};