#include "TransformationMatrix.h"

TransformationMatrix::TransformationMatrix() {};

TransformationMatrix::~TransformationMatrix() {};

void TransformationMatrix::Initialize() {
	dxCommon_ = DirectXCommon::GetInstance();
	//wvpResourceの作成
	wvpResource_ = dxCommon_->CreateBufferResource(sizeof(ConstBufferDataTransformationMatrix));
}

void TransformationMatrix::Map(const WorldTransform& worldTransform, const ViewProjection& viewProjection) {
	//ワールドビュープロジェクション行列の計算
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldTransform.matWorld_, Multiply(viewProjection.matView_, viewProjection.matProjection_));
	//wvpResourceに書き込む
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	wvpData_->WVP = worldViewProjectionMatrix;
	wvpData_->World = worldTransform.matWorld_;
	wvpResource_->Unmap(0, nullptr);
}

void TransformationMatrix::SetGraphicsCommand() {
	//wvpResourceの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
}