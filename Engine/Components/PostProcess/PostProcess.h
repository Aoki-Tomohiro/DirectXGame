#pragma once
#include "DirectXCommon/DirectXCommon.h"
#include "TextureManager/TextureManager.h"
#include "MathFunction.h"
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

class PostProcess {
public:
	struct VertexPosUV {
		Vector4 pos;
		Vector2 texcoord;
	};

	struct GuassianBlurData {
		int32_t textureWidth;
		int32_t textureHeight;
		float padding[2];
		float weight[8];
	};

	struct BloomData {
		//フラグ
		bool enable;
		float padding[3];
	};

	struct FogData {
		//フラグ
		bool enable;
		//スケール
		float scale;
		//減衰率
		float attenuationRate;
	};

	struct DofData {
		//フラグ
		bool enable;
		float padding[3];
	};

	struct LensDistortionData {
		//フラグ
		bool enable;
		//歪みのきつさ
		float tightness;
		//歪みの強さ
		float strength;
	};

	struct VignetteData {
		//フラグ
		bool enable;
		//強度
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
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画後処理
	/// </summary>
	void PostDraw();

	/// <summary>
	/// 最終パス描画処理
	/// </summary>
	void Draw();

private:
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
	GuassianBlurData* gaussianBlurData_ = nullptr;
	//Bloom用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> bloomResource_ = nullptr;
	//書き込み用
	BloomData* bloomData_ = nullptr;
	//フォグ用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> fogResource_ = nullptr;
	//書き込み用
	FogData* fogData_ = nullptr;
	//Dof用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> dofResource_ = nullptr;
	//書き込み用
	DofData* dofData_ = nullptr;
	//LensDistortion用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> lensDistortionResource_ = nullptr;
	//書き込み用
	LensDistortionData* lensDistortionData_ = nullptr;
	//ビネット用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vignetteResource_ = nullptr;
	//書き込み用
	VignetteData* vignetteData_ = nullptr;

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

public:
	//ポストプロセスの有効フラグ
	bool isActive = false;

	//Bloomの有効フラグ
	bool isBloomActive = false;

	//Fogの調整項目
	bool isFogActive = false;
	//スケール
	float fogScale = 0.5f;
	//減衰率
	float fogAttenuationRate = 2.0f;

	//Dofの調整項目
	bool isDofActive = false;

	//LensDistortionの調整項目
	bool isLensDistortionActive = false;
	//歪みのきつさ
	float lensDistortionTightness = 2.5f;
	//歪みの強さ
	float lensDistortionStrength  = -0.1f;

	//Vignetteの調整項目
	bool isVignetteActive = false;
	//強度
	float vignetteIntensity = 1.0f;
};