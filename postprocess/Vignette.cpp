#include "Vignette.h"

Vignette::Vignette() {};

Vignette::~Vignette() {};

void Vignette::Initialize() {
	dxCommon_ = DirectXCommon::GetInstance();
	//ビネット用のリソースの作成
	vignetteResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(ConstBufferDataVignette));
	//ビネットのリソースに書き込む
	vignetteResource_->Map(0, nullptr, reinterpret_cast<void**>(&vignetteData_));
	vignetteData_->enable = enable_;
	vignetteData_->intensity = intensity_;
	vignetteResource_->Unmap(0, nullptr);
}

void Vignette::Update() {
	//ビネットのリソースに書き込む
	vignetteResource_->Map(0, nullptr, reinterpret_cast<void**>(&vignetteData_));
	vignetteData_->enable = enable_;
	vignetteData_->intensity = intensity_;
	vignetteResource_->Unmap(0, nullptr);
}

void Vignette::SetGraphicsCommand() {
	//ビネット用のリソースの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(6, vignetteResource_->GetGPUVirtualAddress());
}