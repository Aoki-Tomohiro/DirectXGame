#include "TransformationMatrix.h"

TransformationMatrix::TransformationMatrix() {};

TransformationMatrix::~TransformationMatrix() {};

void TransformationMatrix::Initialize() {
	//wvpResourceの作成
	wvpResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ConstBufferDataTransformationMatrix));
}

void TransformationMatrix::Update(const WorldTransform& worldTransform, const ViewProjection& viewProjection) {
	//ワールドビュープロジェクション行列の計算
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldTransform.matWorld_, Multiply(viewProjection.matView_, viewProjection.matProjection_));
	//wvpResourceに書き込む
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	wvpData_->WVP = worldViewProjectionMatrix;
	wvpData_->World = worldTransform.matWorld_;
	wvpResource_->Unmap(0, nullptr);
}

void TransformationMatrix::SetGraphicsCommand(UINT rootParameterIndex) {
	//wvpResourceの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(rootParameterIndex, wvpResource_->GetGPUVirtualAddress());
}