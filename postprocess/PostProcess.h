#pragma once
#include "DirectXCommon.h"
#include "Vertex.h"
#include "Fog.h"
#include "Dof.h"
#include "LensDistortion.h"
#include "Vignette.h"
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

class PostProcess {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	PostProcess();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~PostProcess();

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
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// フォグデータを取得
	/// </summary>
	/// <returns></returns>
	Fog* GetFog() { return fog_.get(); };

	/// <summary>
	/// DofDataを取得
	/// </summary>
	/// <returns></returns>
	Dof* GetDof() { return dof_.get(); };

	/// <summary>
	/// LensDistortionDataを取得
	/// </summary>
	/// <returns></returns>
	LensDistortion* GetLensDistortion() { return lensDistortion_.get(); };

	/// <summary>
	/// VignetteDataを取得
	/// </summary>
	/// <returns></returns>
	Vignette* GetVignette() { return vignette_.get(); };

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
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;
	//ビューポート
	D3D12_VIEWPORT viewport_{};
	//シザー矩形
	D3D12_RECT scissorRect_{};
	//メッシュ
	std::vector<VertexData> vertices_;
	std::unique_ptr<Vertex> vertex_ = nullptr;
	//Fog
	std::unique_ptr<Fog> fog_ = nullptr;
	//Dof
	std::unique_ptr<Dof> dof_ = nullptr;
	//LensDistortion
	std::unique_ptr<LensDistortion> lensDistortion_ = nullptr;
	//ビネット
	std::unique_ptr<Vignette> vignette_ = nullptr;
};