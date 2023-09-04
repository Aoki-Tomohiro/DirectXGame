#pragma once
#include "DirectXCommon/DirectXCommon.h"
#include "MathFunction.h"

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

class Mesh {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Mesh();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Mesh();

	/// <summary>
	/// メッシュの作成
	/// </summary>
	/// <param name="dxCommon"></param>
	/// <param name="vertices"></param>
	void Create(const std::vector<VertexData>& vertices);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private:
	//VertexBuffer
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	//VertexBufferView
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	//頂点データ
	std::vector<VertexData> vertices_{};
	//書き込み用
	VertexData* vertexData = nullptr;
};