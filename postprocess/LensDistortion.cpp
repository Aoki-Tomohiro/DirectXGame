#include "LensDistortion.h"

LensDistortion::LensDistortion() {};

LensDistortion::~LensDistortion() {};

void LensDistortion::Initialize() {
	dxCommon_ = DirectXCommon::GetInstance();
	//LensDistortion用のリソースを作成
	LensDistortionResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(ConstBufferDataLensDistortion));
	//LensDistortion用のリソースに書き込む
	LensDistortionResource_->Map(0, nullptr, reinterpret_cast<void**>(&lensDistortionData_));
	lensDistortionData_->enable = enable_;
	lensDistortionData_->tightness = tightness_;
	lensDistortionData_->strength = strength_;
	LensDistortionResource_->Unmap(0, nullptr);
}

void LensDistortion::Update() {
	//LensDistortion用のリソースに書き込む
	LensDistortionResource_->Map(0, nullptr, reinterpret_cast<void**>(&lensDistortionData_));
	lensDistortionData_->enable = enable_;
	lensDistortionData_->tightness = tightness_;
	lensDistortionData_->strength = strength_;
	LensDistortionResource_->Unmap(0, nullptr);
}

void LensDistortion::SetGraphicsCommand() {
	//フォグ用のリソースの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(5, LensDistortionResource_->GetGPUVirtualAddress());
}