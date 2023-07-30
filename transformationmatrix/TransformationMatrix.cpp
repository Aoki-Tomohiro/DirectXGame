#include "TransformationMatrix.h"

TransformationMatrix::TransformationMatrix() {};

TransformationMatrix::~TransformationMatrix() {};

void TransformationMatrix::Initialize() {
	dxCommon_ = DirectXCommon::GetInstance();
	//wvpResourceの作成
	wvpResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(ConstBufferDataTransformationMatrix));
	//wvpResourceに書き込む
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	wvpData_->WVP = MakeIdentity4x4();
	wvpData_->World = MakeIdentity4x4();
	wvpResource_->Unmap(0, nullptr);
}

void TransformationMatrix::Update() {
	//ワールドビュープロジェクション行列を作成
	worldViewProjectionMatrix_ = Multiply(worldMatrix_, Multiply(viewMatrix_, projectionMatrix_));
	//wvpResourceに書き込む
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	wvpData_->WVP = worldViewProjectionMatrix_;
	wvpData_->World = worldMatrix_;
	wvpResource_->Unmap(0, nullptr);
}

void TransformationMatrix::SetGraphicsCommand() {
	//wvpResourceの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
}