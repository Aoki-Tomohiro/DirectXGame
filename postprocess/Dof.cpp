#include "Dof.h"

Dof::Dof() {};

Dof::~Dof() {};

void Dof::Initialize() {
	dxCommon_ = DirectXCommon::GetInstance();
	//Dof用のリソースを作成
	dofResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(ConstBufferDataDof));
	//Dof用のリソースに書き込む
	dofResource_->Map(0, nullptr, reinterpret_cast<void**>(&dofData_));
	dofData_->enable = enable_;
	dofResource_->Unmap(0, nullptr);
}

void Dof::Update() {
	//Dof用のリソースに書き込む
	dofResource_->Map(0, nullptr, reinterpret_cast<void**>(&dofData_));
	dofData_->enable = enable_;
	dofResource_->Unmap(0, nullptr);
}

void Dof::SetGraphicsCommand() {
	//Dof用のリソースの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(4, dofResource_->GetGPUVirtualAddress());
}