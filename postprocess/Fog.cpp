#include "Fog.h"

Fog::Fog() {};

Fog::~Fog() {};

void Fog::Initialize() {
	dxCommon_ = DirectXCommon::GetInstance();
	//フォグ用のリソースを作成
	fogResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(ConstBufferDataFog));
	//フォグ用のリソースに書き込む
	fogResource_->Map(0, nullptr, reinterpret_cast<void**>(&fogData_));
	fogData_->enable = enable_;
	fogData_->scale = scale_;
	fogData_->attenuationRate = attenuationRate_;
	fogResource_->Unmap(0, nullptr);
}

void Fog::Update() {
	//フォグ用のリソースに書き込む
	fogResource_->Map(0, nullptr, reinterpret_cast<void**>(&fogData_));
	fogData_->enable = enable_;
	fogData_->scale = scale_;
	fogData_->attenuationRate = attenuationRate_;
	fogResource_->Unmap(0, nullptr);
}

void Fog::SetGraphicsCommand() {
	//フォグ用のリソースの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, fogResource_->GetGPUVirtualAddress());
}