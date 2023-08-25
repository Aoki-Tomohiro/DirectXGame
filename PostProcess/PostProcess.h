#pragma once
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "MathFunction.h"
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

class PostProcess {
public:
	struct VertexPosUV {
		Vector4 pos;
		Vector2 texcoord;
	};

	struct ConstBufferDataGaussianBlur {
		int32_t textureWidth;
		int32_t textureHeight;
		float padding[2];
		float weight[8];
	};

	struct ConstBufferDataBloom {
		bool enable;
		float padding[3];
	};

	struct ConstBufferDataFog {
		bool enable;
		float scale;
		float attenuationRate;
	};

	struct ConstBufferDataDof {
		bool enable;
		float padding[3];
	};

	struct ConstBufferDataLensDistortion {
		bool enable;
		float tightness;
		float strength;
	};

	struct ConstBufferDataVignette {
		bool enable;
		float intensity;
	};

	/// <summary>
	/// シングルトンインスタンスを取得
	/// </summary>
	/// <returns></returns>
	static PostProcess* GetInstance();

	/// <summary>
	/// インスタンスを削除
	/// </summary>
	static void DeleteInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
    /// DXCompilerの初期化
    /// </summary>
	void InitializeDxcCompiler();

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
	/// 二パス目のパイプラインステートの作成
	/// </summary>
	void CreateSecondPassPipelineStateObject();

	/// <summary>
	/// ブラー用のパイプラインステートの作成
	/// </summary>
	void CreateBlurPipelineStateObject();

	/// <summary>
	/// 最終パスのパイプラインステートの作成
	/// </summary>
	void CreateFinalPassPipelineStateObject();

	/// <summary>
	/// マルチパス用のレンダーターゲットビューの作成
	/// </summary>
	uint32_t CreateMultiPassRenderTargetView(const Microsoft::WRL::ComPtr<ID3D12Resource>& resource, DXGI_FORMAT format);

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画後処理
	/// </summary>
	void PostDraw();

	/// <summary>
	/// 二パス目描画前処理
	/// </summary>
	void PreSecondPassDraw();

	/// <summary>
	/// 二パス目描画後処理
	/// </summary>
	void PostSecondPassDraw();

	/// <summary>
	/// 二パス目描画処理
	/// </summary>
	void SecondPassDraw();

	/// <summary>
	/// 横ぼかし前処理
	/// </summary>
	void PreHorizontalBlur();

	/// <summary>
	/// 横ぼかし後処理
	/// </summary>
	void PostHorizontalBlur();

	/// <summary>
	/// 縦ぼかし前処理
	/// </summary>
	void PreVerticalBlur();

	/// <summary>
	/// 縦ぼかし後処理
	/// </summary>
	void PostVerticalBlur();

	/// <summary>
	/// 横縮小ぼかし前処理
	/// </summary>
	void PreHorizontalShrinkBlur();

	/// <summary>
	/// 横縮小ぼかし後処理
	/// </summary>
	void PostHorizontalShrinkBlur();

	/// <summary>
	/// 縦縮小ぼかし前処理
	/// </summary>
	void PreVerticalShrinkBlur();

	/// <summary>
	/// 縦縮小ぼかし後処理
	/// </summary>
	void PostVerticalShrinkBlur();

	/// <summary>
	/// ぼかし処理
	/// </summary>
	void Blur(const Microsoft::WRL::ComPtr<ID3D12PipelineState>& pipelineState, uint32_t blurSRVIndex, uint32_t highIntensityBlurSRVIndex);

	/// <summary>
	/// 最終パス描画処理
	/// </summary>
	void Draw();

private:
	//インスタンス
	static PostProcess* instance;
	//DirectXCommon
	DirectXCommon* dxCommon_ = nullptr;
	//DXCompiler
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> secondPassRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> blurRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> finalPassRootSignature_;
	//パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> secondPassPipelineState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> horizontalBlurPipelineState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> verticalBlurPipelineState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> finalPassPipelineState_;
	//コマンドリスト
	ID3D12GraphicsCommandList* commandList_;

	//頂点
	std::vector<VertexPosUV> vertices_{};
	//頂点バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	//頂点データの書き込み用
	VertexPosUV* vertexData_ = nullptr;

	//マルチパス用のRTVディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> multiPassRTVDescriptorHeap_ = nullptr;
	uint32_t rtvIndex_ = -1;
	//マルチパス用のDSVディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> multiPassDSVDescriptorHeap_ = nullptr;

	//マルチパス用の深度バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;
	//ブラー用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> gaussianBlurResource_ = nullptr;
	//書き込み用
	ConstBufferDataGaussianBlur* gaussianBlurData_ = nullptr;
	//分散値
	float s_ = 5.0f;

	//一パス目のディスクリプタハンドル
	uint32_t firstPassRTVIndex_ = 0;
	uint32_t firstPassSRVIndex_ = 0;
	//線形深度のディスクリプタハンドル
	uint32_t linearDepthRTVIndex_ = 0;
	uint32_t linearDepthSRVIndex_ = 0;
	//二パス目のディスクリプタハンドル
	uint32_t secondPassRTVIndex_ = 0;
	uint32_t secondPassSRVIndex_ = 0;
	//高輝度のディスクリプタハンドル
	uint32_t highIntensityRTVIndex_ = 0;
	uint32_t highIntensitySRVIndex_ = 0;
	//ぼかしのディスクリプタハンドル
	uint32_t blurRTVIndex_[2] = {};
	uint32_t blurSRVIndex_[2] = {};
	//高輝度ぼかしのディスクリプタハンドル
	uint32_t highIntensityBlurRTVIndex_[2] = {};
	uint32_t highIntensityBlurSRVIndex_[2] = {};
	//縮小ぼかしのディスクリプタハンドル
	uint32_t shrinkBlurRTVIndex_[2] = {};
	uint32_t shrinkBlurSRVIndex_[2] = {};
	//高輝度縮小ぼかしのディスクリプタハンドル
	uint32_t highIntensityShrinkBlurRTVIndex_[2] = {};
	uint32_t highIntensityShrinkBlurSRVIndex_[2] = {};

	//Bloom用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> bloomResource_ = nullptr;
	//書き込み用
	ConstBufferDataBloom* bloomData_ = nullptr;
	//フォグ用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> fogResource_ = nullptr;
	//書き込み用
	ConstBufferDataFog* fogData_ = nullptr;
	//Dof用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> dofResource_ = nullptr;
	//書き込み用
	ConstBufferDataDof* dofData_ = nullptr;
	//LensDistortion用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> lensDistortionResource_ = nullptr;
	//書き込み用
	ConstBufferDataLensDistortion* lensDistortionData_ = nullptr;
	//ビネット用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vignetteResource_ = nullptr;
	//書き込み用
	ConstBufferDataVignette* vignetteData_ = nullptr;
};