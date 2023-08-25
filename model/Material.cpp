#include "Material.h"

Material::Material() {};

Material::~Material() {};

void Material::Create() {
	//マテリアルリソースの作成
	materialResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ConstBufferDataMaterial));
	//マテリアルリソースに書き込む
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = color_;
	materialData_->lightingMethod = lightingMethod_;
	materialData_->enableLighting = enableLighting_;
	materialData_->uvTransform = MakeIdentity4x4();
	materialResource_->Unmap(0, nullptr);
}

void Material::Update() {
	//uvTransform
	Transform uvTransform = { {scale_.x,scale_.y,1.0f},{0.0f,0.0f,rotation_},{translation_.x,translation_.y,0.0f} };
	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransform.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransform.translate));
	//マテリアルリソースに書き込む
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = color_;
	materialData_->enableLighting = enableLighting_;
	materialData_->lightingMethod = lightingMethod_;
	materialData_->uvTransform = uvTransformMatrix;
	materialResource_->Unmap(0, nullptr);
}

void Material::SetGraphicsCommand(UINT rootParameterIndex) {
	//マテリアルリソースの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(rootParameterIndex, materialResource_->GetGPUVirtualAddress());
}