#include "DirectionalLight.h"

DirectionalLight::DirectionalLight() {};

DirectionalLight::~DirectionalLight() {};

void DirectionalLight::Initialize() {
	//lightingResourceの作成
	lightingResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ConstBufferDataDirectionalLight));
	//lightingResourceに書き込む
	lightingResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	directionalLightData_->color = color_;
	directionalLightData_->direction = direction_;
	directionalLightData_->intensity = intensity_;
	lightingResource_->Unmap(0, nullptr);
}

void DirectionalLight::Update() {
	//lightingResourceに書き込む
	lightingResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	directionalLightData_->color = color_;
	directionalLightData_->direction = direction_;
	directionalLightData_->intensity = intensity_;
	lightingResource_->Unmap(0, nullptr);
}

void DirectionalLight::SetGraphicsCommand(UINT rootParameterIndex) {
	//lightingResourceの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(rootParameterIndex, lightingResource_->GetGPUVirtualAddress());
}