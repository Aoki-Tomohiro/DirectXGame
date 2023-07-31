#pragma once
#include "DirectXCommon.h"
#include "Vertex.h"
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

struct ConstBufferDataGaussianBlur {
	float weight[8];
};

class GaussianBlur {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	GaussianBlur();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GaussianBlur();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// DXCCompilerの初期化
	/// </summary>
	void InitializeDXCCompiler();

	/// <summary>
	/// シェーダーの読み込み
	/// </summary>
	/// <param name="filePath"></param>
	/// <param name="profile"></param>
	/// <param name="dxcUtils"></param>
	/// <param name="dxcCompiler"></param>
	/// <param name="includeHandler"></param>
	/// <returns></returns>
	IDxcBlob* CompileShader(//CompilerするShaderファイルへのパス
		const std::wstring& filePath,
		//compilerに使用するProfile
		const wchar_t* profile,
		//初期化で生成したものを3つ
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler);

	/// <summary>
	/// パイプラインステートの作成
	/// </summary>
	void CreatePipelineStateObject();

	/// <summary>
	/// ビューポートの作成
	/// </summary>
	void CreateViewport();

	/// <summary>
	/// シザーレクトの作成
	/// </summary>
	void CreateScissorRect();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 横ぼかし
	/// </summary>
	void HorizontalBlur();

	/// <summary>
	/// 縦ぼかし
	/// </summary>
	void VerticalBlur();

	/// <summary>
	/// 縮小横ぼかし
	/// </summary>
	void HorizontalShrinkBlur();

	/// <summary>
	/// 縮小縦ぼかし
	/// </summary>
	void VerticalShrinkBlur();

private:
	DirectXCommon* dxCommon_ = nullptr;
	//dxcCompiler
	IDxcUtils* dxcUtils_ = nullptr;
	IDxcCompiler3* dxcCompiler_ = nullptr;
	IDxcIncludeHandler* includeHandler_ = nullptr;
	//RootSignature
	ID3DBlob* signatureBlob_ = nullptr;
	ID3DBlob* errorBlob_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	//Shaderコンパイル
	IDxcBlob* vertexShaderBlob_ = nullptr;
	IDxcBlob* pixelShaderBlob_ = nullptr;
	//PSO
	Microsoft::WRL::ComPtr<ID3D12PipelineState> horizontalBlurGraphicsPipelineState_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> verticalBlurGraphicsPipelineState_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> horizontalShrinkBlurGraphicsPipelineState_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> verticalShrinkBlurGraphicsPipelineState_ = nullptr;
	//ビューポート
	D3D12_VIEWPORT viewport_{};
	//シザー矩形
	D3D12_RECT scissorRect_{};
	//メッシュ
	std::vector<VertexData> vertices_;
	std::unique_ptr<Vertex> vertex_ = nullptr;
	//ガウシアンぼかし用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> gaussianBlurResource_ = nullptr;
	//書き込み用
	ConstBufferDataGaussianBlur* gaussianBlurData_ = nullptr;
	//分散値
	float s_ = 5.0f;
};