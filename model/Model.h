#pragma once
#include "DirectXCommon.h"
#include "Vertex.h"
#include "Material.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "TransformationMatrix.h"
#include "MathFunction.h"
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

struct MaterialData {
	std::string textureFilePath;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

class Model {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	static void Initialize();

	/// <summary>
	/// DXCCompilerの初期化
	/// </summary>
	static void InitializeDXCCompiler();

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
	/// ビューポートの作成
	/// </summary>
	static void CreateViewport();

	/// <summary>
	/// シザーレクトの作成
	/// </summary>
	static void CreateScissorRect();

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
	void Create(const std::vector<VertexData>& vertices,Texture* texture);

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
	void Draw(TransformationMatrix* transformationMatrix);

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
	//DirectXCommon
	static DirectXCommon* dxCommon_;
	//dxcCompiler
	static IDxcUtils* dxcUtils_;
	static IDxcCompiler3* dxcCompiler_;
	static IDxcIncludeHandler* includeHandler_;
	//RootSignature
	static ID3DBlob* signatureBlob_;
	static ID3DBlob* errorBlob_;
	static Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	//Shaderコンパイル
	static IDxcBlob* vertexShaderBlob_;
	static IDxcBlob* pixelShaderBlob_;
	//PSO
	static Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
	//ビューポート
	static D3D12_VIEWPORT viewport_;
	//シザー矩形
	static D3D12_RECT scissorRect_;
	//ライティング
	static std::unique_ptr<DirectionalLight> directionalLight_;
	//モデルデータ
	ModelData modelData_;
	//メッシュ
	std::unique_ptr<Vertex> vertex_ = nullptr;
	//マテリアル
	std::unique_ptr<Material> material_ = nullptr;
	//テクスチャ
	std::unique_ptr<Texture> texture_ = nullptr;
};