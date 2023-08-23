#include "Vertex.h"

Vertex::Vertex() {}

Vertex::~Vertex() {};

void Vertex::Create(const std::vector<VertexData>& vertices) {
	dxCommon_ = DirectXCommon::GetInstance();
	//頂点情報を取得
	vertices_ = vertices;
	//頂点バッファを作成
	vertexBuffer_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * vertices_.size());
	//頂点バッファビューを作成
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();//リソースの先頭のアドレスから使う
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * vertices_.size());//使用するリソースのサイズは頂点のサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	//頂点バッファにデータを書き込む
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));//書き込むためのアドレスを取得
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());
	vertexBuffer_->Unmap(0, nullptr);
}

void Vertex::Draw() {
	//VBVを設定
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	//形状を設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//描画
	dxCommon_->GetCommandList()->DrawInstanced(UINT(vertices_.size()), 1, 0, 0);
}