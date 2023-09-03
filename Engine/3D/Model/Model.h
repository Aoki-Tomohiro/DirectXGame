#pragma once
#include "DirectXCommon/DirectXCommon.h"
#include "TextureManager/TextureManager.h"
#include "TransformationMatrix/TransformationMatrix.h"
#include "Light/DirectionalLight.h"
#include "Vertex.h"
#include "Material.h"
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
	/// 球の作成
	/// </summary>
	void CreateSphere();

	/// <summary>
	/// OBJからモデルの作成
	/// </summary>
	void CreateFromOBJ(const std::string& directoryPath, const std::string& filename);

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
	DirectionalLight* GetDirectionalLight() { return directionalLight_.get(); };

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
	void CreatePipelineStateObject();

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
	//ライティング
	std::unique_ptr<DirectionalLight> directionalLight_ = nullptr;
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