#pragma once
#include "DirectXCommon/DirectXCommon.h"
#include "TextureManager/TextureManager.h"
#include "MathFunction.h"
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

enum BlendMode {
	kNone,
	kNormal,
	kAdd,
	kSubtract,
};

class Sprite{
public:
	enum class RootParameterIndex {
		TransformationMatrix,
		Material,
		Texture
	};

	struct VertexPosUV {
		Vector4 pos;
		Vector2 texcoord;
	};

	struct ConstBufferDataWVP {
		Matrix4x4 WVP;
	};

	struct ConstBufferDataMaterial {
		Vector4 color;
		Matrix4x4 uvTransform;
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	Sprite();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Sprite();

	/// <summary>
	/// スプライト作成
	/// </summary>
	/// <param name="textureHandle"></param>
	/// <param name="position"></param>
	void Create(uint32_t textureHandle, Vector2 position, BlendMode blendMode);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 座標を設定
	/// </summary>
	/// <param name="translation"></param>
	void SetTranslation(const Vector2& translation) { translation_ = translation; };

	/// <summary>
	/// 座標を取得
	/// </summary>
	/// <returns></returns>
	const Vector2& GetTranslation() { return translation_; };

	/// <summary>
	/// 角度を設定
	/// </summary>
	/// <param name="rotation"></param>
	void SetRotation(const float& rotation) { rotation_ = rotation; };

	/// <summary>
	/// 角度を取得
	/// </summary>
	/// <returns></returns>
	const float& GetRotation() { return rotation_; };

	/// <summary>
	/// スケールを設定
	/// </summary>
	/// <param name="scale"></param>
	void SetScale(const Vector2& scale) { scale_ = scale; };

	/// <summary>
	/// スケールを取得
	/// </summary>
	/// <returns></returns>
	const Vector2& GetScale() { return scale_; };

	/// <summary>
	/// 色を設定
	/// </summary>
	/// <param name="color"></param>
	void SetColor(const Vector4& color) { color_ = color; };

	/// <summary>
	/// 色を取得
	/// </summary>
	/// <returns></returns>
	const Vector4& GetColor() { return color_; };

	/// <summary>
	/// uv座標を設定
	/// </summary>
	/// <param name="uvTranslation"></param>
	void SetUVTranslation(const Vector2& uvTranslation) { uvTranslation_ = uvTranslation; };

	/// <summary>
	/// uv座標を取得
	/// </summary>
	/// <returns></returns>
	const Vector2 GetUVTranslation() { return uvTranslation_; };

	/// <summary>
	/// uv角度を設定
	/// </summary>
	/// <param name="uvRotation"></param>
	void SetUVRotation(const float& uvRotation) { uvRotation_ = uvRotation; };

	/// <summary>
	/// uv角度を取得
	/// </summary>
	/// <returns></returns>
	const float GetUVRotation() { return uvRotation_; };

	/// <summary>
	/// uvスケールを設定
	/// </summary>
	/// <param name="uvScale"></param>
	void SetUVScale(const Vector2& uvScale) { uvScale_ = uvScale; };

	/// <summary>
	/// uvスケールを取得
	/// </summary>
	/// <returns></returns>
	const Vector2 GetUVScale() { return uvScale_; };

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
	/// パイプラインステートの作成
	/// </summary>
	void CreatePipelineStateObject(BlendMode blendMode);

	/// <summary>
	/// マッピング
	/// </summary>
	void Map();

private:
	//DXCompiler
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	//パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_ = nullptr;
	//コマンドリスト
	ID3D12GraphicsCommandList* commandList_ = nullptr;
	//プロジェクション行列
	Matrix4x4 matProjection_{};
	//頂点
	std::vector<VertexPosUV> vertices_{};
	//頂点バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	//頂点データの書き込み用
	VertexPosUV* vertexData_ = nullptr;
	//WVP用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> WVPResource_ = nullptr;
	//WVPの書き込み用
	ConstBufferDataWVP* wvpData_ = nullptr;
	//マテリアル用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> MaterialResource_ = nullptr;
	//マテリアルの書き込み用
	ConstBufferDataMaterial* materialData_ = nullptr;
	//テクスチャハンドル
	uint32_t textureHandle_ = 0;
	//リソース設定
	D3D12_RESOURCE_DESC resourceDesc_{};
	//座標
	Vector2 translation_{ 0.0f,0.0f };
	//角度
	float rotation_ = 0.0f;
	//スケール
	Vector2 scale_{ 1.0f,1.0f };
	//色
	Vector4 color_{ 1.0f,1.0f,1.0f,1.0f };
	//uv座標
	Vector2 uvTranslation_{0.0f,0.0f};
	//uv角度
	float uvRotation_ = 0.0f;;
	//uvスケール
	Vector2 uvScale_{ 1.0f,1.0f };
};

