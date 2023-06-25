#pragma once
#include "DirectX.h"
#include "math/Math.h"
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

class Model {
public:
	~Model();
	void Initialize(DirectXCommon* directX);
	void InitializeDXCCompiler();
	IDxcBlob* CompileShader(//CompilerするShaderファイルへのパス
		const std::wstring& filePath,
		//compilerに使用するProfile
		const wchar_t* profile,
		//初期化で生成したものを3つ
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler);
	void CreatePipelineStateObject();
	ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
	void CreateVertexData(ID3D12Resource* vertexResource, D3D12_VERTEX_BUFFER_VIEW& vertexBufferView, UINT sizeInBytes, VertexData* vertexData, uint32_t vertexCount);
	void CreateMaterialData(ID3D12Resource* materialResource, Material* color);
	void UpdateMatrix(ID3D12Resource* WVPResource, TransformationMatrix matrix);
	void CreateViewport();
	void CreateScissorRect();
	void Draw(ID3D12Resource* resource, D3D12_VERTEX_BUFFER_VIEW vertexBufferView, VertexData* vertexData, UINT sizeInBytes, uint32_t vertexCount, ID3D12Resource* materialResource, Material* color, ID3D12Resource* WVPResource, bool useMonsterBall, ID3D12Resource* lightingResource);
private:
	//DirectX
	DirectXCommon* directX_ = nullptr;
	//dxcCompiler
	IDxcUtils* dxcUtils_ = nullptr;
	IDxcCompiler3* dxcCompiler_ = nullptr;
	IDxcIncludeHandler* includeHandler_ = nullptr;
	//RootSignature
	ID3DBlob* signatureBlob_ = nullptr;
	ID3DBlob* errorBlob_ = nullptr;
	ID3D12RootSignature* rootSignature_ = nullptr;
	//Shaderコンパイル
	IDxcBlob* vertexShaderBlob_{};
	IDxcBlob* pixelShaderBlob_{};
	//PSO
	ID3D12PipelineState* graphicsPipelineState_ = nullptr;
	//ビューポート
	D3D12_VIEWPORT viewport_{};
	//シザー矩形
	D3D12_RECT scissorRect_{};
};