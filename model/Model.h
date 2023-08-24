#pragma once
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "Vertex.h"
#include "Material.h"
#include "DirectionalLight.h"
#include "TransformationMatrix.h"
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

class Model {
public:
	//ルートパラメーターの番号
	enum class RootParameterIndex {
		Material,
		TransformationMatrix,
		Texture,
		DirectionalLight
	};

	//マテリアルデータ構造体
	struct MaterialData {
		std::string textureFilePath;
	};

	//モデルデータ構造体
	struct ModelData {
		std::vector<VertexData> vertices;
		MaterialData material;
	};

	/// <summary>
	/// 初期化
	/// </summary>
	static void Initialize();

	/// <summary>
	/// 解放処理
	/// </summary>
	static void Delete();

	/// <summary>
	/// DXCompilerの初期化
	/// </summary>
	static void InitializeDxcCompiler();

	/// <summary>
	/// シェーダーの読み込み
	/// </summary>
	/// <param name="filePath"></param>
	/// <param name="profile"></param>
	/// <param name="dxcUtils"></param>
	/// <param name="dxcCompiler"></param>
	/// <param name="includeHandler"></param>
	/// <returns></returns>
	static IDxcBlob* CompileShader(//CompilerするShaderファイルへのパス
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
	static void CreatePipelineStateObject();

	/// <summary>
	/// コンストラクタ
	/// </summary>
	Model();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Model();

	/// <summary>
	/// モデルの作成
	/// </summary>
	void Create(const std::vector<VertexData>& vertices);

	/// <summary>
	/// OBJからモデルの作成
	/// </summary>
	void CreateFromOBJ(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// Objファイルの読み込み
	/// </summary>
	/// <param name="directoryPath"></param>
	/// <param name="filename"></param>
	/// <returns></returns>
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// mtlファイルの読み込み
	/// </summary>
	/// <param name="directoryPath"></param>
	/// <param name="filename"></param>
	/// <returns></returns>
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	/// <summary>
    /// 描画
    /// </summary>
	void Draw(const WorldTransform& worldTransform, const ViewProjection& viewProjection);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(const WorldTransform& worldTransform, const ViewProjection& viewProjection, uint32_t textureHandle);

	/// <summary>
	/// マテリアルを取得
	/// </summary>
	/// <returns></returns>
	Material* GetMaterial() { return material_.get(); };

	/// <summary>
	/// directionalLightを取得
	/// </summary>
	/// <returns></returns>
	DirectionalLight* GetDirectionalLight() { return sDirectionalLight_.get(); };

private:
	//DXCompiler
	static Microsoft::WRL::ComPtr<IDxcUtils> sDxcUtils_;
	static Microsoft::WRL::ComPtr<IDxcCompiler3> sDxcCompiler_;
	static Microsoft::WRL::ComPtr<IDxcIncludeHandler> sIncludeHandler_;
	//ルートシグネチャ
	static Microsoft::WRL::ComPtr<ID3D12RootSignature> sRootSignature_;
	//パイプラインステート
	static Microsoft::WRL::ComPtr<ID3D12PipelineState> sPipelineState_;
	//コマンドリスト
	static ID3D12GraphicsCommandList* sCommandList_;
	//ライティング
	static std::unique_ptr<DirectionalLight> sDirectionalLight_;
	//モデルデータ
	ModelData modelData_;
	//メッシュ
	std::unique_ptr<Vertex> vertex_ = nullptr;
	//マテリアル
	std::unique_ptr<Material> material_ = nullptr;
	//行列
	std::unique_ptr<TransformationMatrix> transformationMatrix_ = nullptr;
	//テクスチャハンドル
	uint32_t textureHandle_ = 0;
};