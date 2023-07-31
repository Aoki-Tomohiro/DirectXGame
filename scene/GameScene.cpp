#include "GameScene.h"
#include <cassert>

GameScene::GameScene() {};

GameScene::~GameScene() {};

void GameScene::Initialize(GameManager* gameManager) {
	//DirectXの取得
	dxCommon_ = DirectXCommon::GetInstance();
	imguiManager_ = ImGuiManager::GetInstance();
	//テクスチャ
	texture_ = std::make_unique<Texture>();
	texture_->Load("resources/uvChecker.png");
	//三角形
	triangle_.push_back(VertexData{ {-0.5f,-0.5f,0.0f,1.0f},{0.0f,1.0f},{0.0f,0.0f,-1.0f} });
	triangle_.push_back(VertexData{ {0.0f,0.5f,0.0f,1.0f },{0.5f,0.0f},{0.0f,0.0f,-1.0f} });
	triangle_.push_back(VertexData{ {0.5f,-0.5f,0.0f,1.0f},{1.0f,1.0f},{0.0f,0.0f,-1.0f} });
	model_ = std::make_unique<Model>();
	model_->Create(triangle_, texture_.get());
	model_->GetMaterial()->enableLighting_ = false;
	transformationMatrix_ = std::make_unique<TransformationMatrix>();
	transformationMatrix_->Initialize();
	//球
	const float pi = 3.14f;
	const uint32_t kSubdivision = 16;
	uint32_t latIndex = 0;
	uint32_t lonIndex = 0;
	//経度分割一つ分の角度φd
	const float kLonEvery = pi * 2.0f / float(kSubdivision);
	//緯度分割一つ分の角度θd
	const float kLatEvery = pi / float(kSubdivision);
	//緯度の方向に分割
	for (latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;//θ
		//経度の方向に分割しながら線を描く
		for (lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;//φ
			//頂点にデータを入力する。基準点a
			sphere_.push_back(VertexData{ {std::cos(lat) * std::cos(lon),std::sin(lat),std::cos(lat) * std::sin(lon),1.0f},
				{ float(lonIndex) / float(kSubdivision),1.0f - float(latIndex) / float(kSubdivision)},
				{std::cos(lat) * std::cos(lon),std::sin(lat),std::cos(lat) * std::sin(lon)} });

			//残りの５頂点も順番に計算して入力していく
			sphere_.push_back(VertexData{ {std::cos(lat + kLatEvery) * std::cos(lon),std::sin(lat + kLatEvery),std::cos(lat + kLatEvery) * std::sin(lon),1.0f},
				{float(lonIndex) / float(kSubdivision),1.0f - float(latIndex + 1) / float(kSubdivision)},
				{std::cos(lat + kLatEvery) * std::cos(lon),std::sin(lat + kLatEvery),std::cos(lat + kLatEvery) * std::sin(lon)} });

			sphere_.push_back(VertexData{ {std::cos(lat) * std::cos(lon + kLonEvery),std::sin(lat),std::cos(lat) * std::sin(lon + kLonEvery),1.0f},
				{float(lonIndex + 1) / float(kSubdivision),1.0f - float(latIndex) / float(kSubdivision)},
				{std::cos(lat) * std::cos(lon + kLonEvery),std::sin(lat),std::cos(lat) * std::sin(lon + kLonEvery)} });

			sphere_.push_back(VertexData{ { std::cos(lat) * std::cos(lon + kLonEvery),std::sin(lat),std::cos(lat) * std::sin(lon + kLonEvery),1.0f},
				{float(lonIndex + 1) / float(kSubdivision),1.0f - float(latIndex) / float(kSubdivision)},
				{std::cos(lat) * std::cos(lon + kLonEvery),std::sin(lat),std::cos(lat) * std::sin(lon + kLonEvery)} });

			sphere_.push_back(VertexData{ {std::cos(lat + kLatEvery) * std::cos(lon),std::sin(lat + kLatEvery),std::cos(lat + kLatEvery) * std::sin(lon),1.0f},
				{float(lonIndex) / float(kSubdivision),1.0f - float(latIndex + 1) / float(kSubdivision)},
				{std::cos(lat + kLatEvery) * std::cos(lon),std::sin(lat + kLatEvery),std::cos(lat + kLatEvery) * std::sin(lon)} });

			sphere_.push_back(VertexData{ {std::cos(lat + kLatEvery) * std::cos(lon + kLonEvery),std::sin(lat + kLatEvery),std::cos(lat + kLatEvery) * std::sin(lon + kLonEvery),1.0f},
				{float(lonIndex + 1) / float(kSubdivision),1.0f - float(latIndex + 1) / float(kSubdivision)},
				{std::cos(lat + kLatEvery) * std::cos(lon + kLonEvery),std::sin(lat + kLatEvery),std::cos(lat + kLatEvery) * std::sin(lon + kLonEvery)} });
		}
	}
	modelSphere_ = std::make_unique<Model>();
	modelSphere_->Create(sphere_, texture_.get());
	transformationMatrixSphere_ = std::make_unique<TransformationMatrix>();
	transformationMatrixSphere_->Initialize();
	//スプライト
	sprite_.push_back(VertexData{ { 0.0f,360.0f,0.0f,1.0f } ,{ 0.0f,1.0f } ,{ 0.0f,0.0f,-1.0f } });//左下
	sprite_.push_back(VertexData{ { 0.0f,0.0f,0.0f,1.0f } ,{ 0.0f,0.0f } ,{ 0.0f,0.0f,-1.0f } });//左上
	sprite_.push_back(VertexData{ { 640.0f,360.0f,0.0f,1.0f } ,{ 1.0f,1.0f } ,{ 0.0f,0.0f,-1.0f } });//右下
	sprite_.push_back(VertexData{ { 0.0f,0.0f,0.0f,1.0f } ,{ 0.0f,0.0f } ,{ 0.0f,0.0f,-1.0f } });//左上
	sprite_.push_back(VertexData{ { 640.0f,0.0f,0.0f,1.0f } ,{ 1.0f,0.0f } ,{ 0.0f,0.0f, -1.0f } });//右上
	sprite_.push_back(VertexData{ { 640.0f,360.0f,0.0f,1.0f } ,{ 1.0f,1.0f } ,{ 0.0f,0.0f,-1.0f } });//右下
	modelSprite_ = std::make_unique<Model>();
	modelSprite_->Create(sprite_, texture_.get());
	modelSprite_->GetMaterial()->enableLighting_ = false;
	transformationMatrixSprite_ = std::make_unique<TransformationMatrix>();
	transformationMatrixSprite_->Initialize();
	//utahTeapot
	modelUtahTeapot_ = std::make_unique<Model>();
	modelUtahTeapot_->CreateFromOBJ("resources", "multiMesh.obj");
	transformationMatrixUtahTeapot_ = std::make_unique<TransformationMatrix>();
	transformationMatrixUtahTeapot_->Initialize();
	//StanfordBunny
	modelStanfordBunny_ = std::make_unique<Model>();
	modelStanfordBunny_->CreateFromOBJ("resources", "bunny.obj");
	transformationMatrixStanfordBunny_ = std::make_unique<TransformationMatrix>();
	transformationMatrixStanfordBunny_->Initialize();
};

void GameScene::Update(GameManager* gameManager) {
	//三角形
	transformationMatrix_->worldMatrix_ = MakeAffineMatrix(transformationMatrix_->transform_.scale, transformationMatrix_->transform_.rotate, transformationMatrix_->transform_.translate);
	transformationMatrix_->cameraMatrix_ = MakeAffineMatrix(transformationMatrix_->cameraTransform_.scale, transformationMatrix_->cameraTransform_.rotate, transformationMatrix_->cameraTransform_.translate);
	transformationMatrix_->viewMatrix_ = Inverse(transformationMatrix_->cameraMatrix_);
	transformationMatrix_->projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, float(dxCommon_->GetWinApp()->kClientWidth) / float(dxCommon_->GetWinApp()->kClientHeight), 0.1f, 100.0f);
	transformationMatrix_->Update();
	model_->GetMaterial()->Update();
	//球
	transformationMatrixSphere_->worldMatrix_ = MakeAffineMatrix(transformationMatrixSphere_->transform_.scale, transformationMatrixSphere_->transform_.rotate, transformationMatrixSphere_->transform_.translate);
	transformationMatrixSphere_->cameraMatrix_ = MakeAffineMatrix(transformationMatrixSphere_->cameraTransform_.scale, transformationMatrixSphere_->cameraTransform_.rotate, transformationMatrixSphere_->cameraTransform_.translate);
	transformationMatrixSphere_->viewMatrix_ = Inverse(transformationMatrixSphere_->cameraMatrix_);
	transformationMatrixSphere_->projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, float(dxCommon_->GetWinApp()->kClientWidth) / float(dxCommon_->GetWinApp()->kClientHeight), 0.1f, 100.0f);
	transformationMatrixSphere_->Update();
	modelSphere_->GetMaterial()->Update();
	//スプライト
	transformationMatrixSprite_->worldMatrix_ = MakeAffineMatrix(transformationMatrixSprite_->transform_.scale, transformationMatrixSprite_->transform_.rotate, transformationMatrixSprite_->transform_.translate);
	transformationMatrixSprite_->cameraMatrix_ = MakeAffineMatrix(transformationMatrixSprite_->cameraTransform_.scale, transformationMatrixSprite_->cameraTransform_.rotate, transformationMatrixSprite_->cameraTransform_.translate);
	transformationMatrixSprite_->viewMatrix_ = MakeIdentity4x4();
	transformationMatrixSprite_->projectionMatrix_ = MakeOrthographicMatrix(0.0f, 0.0f, float(dxCommon_->GetWinApp()->kClientWidth), float(dxCommon_->GetWinApp()->kClientHeight), 0.0f, 100.0f);
	transformationMatrixSprite_->Update();
	modelSprite_->GetMaterial()->Update();
	//utahTeapot
	transformationMatrixUtahTeapot_->worldMatrix_ = MakeAffineMatrix(transformationMatrixUtahTeapot_->transform_.scale, transformationMatrixUtahTeapot_->transform_.rotate, transformationMatrixUtahTeapot_->transform_.translate);
	transformationMatrixUtahTeapot_->cameraMatrix_ = MakeAffineMatrix(transformationMatrixUtahTeapot_->cameraTransform_.scale, transformationMatrixUtahTeapot_->cameraTransform_.rotate, transformationMatrixUtahTeapot_->cameraTransform_.translate);
	transformationMatrixUtahTeapot_->viewMatrix_ = Inverse(transformationMatrixUtahTeapot_->cameraMatrix_);
	transformationMatrixUtahTeapot_->projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, float(dxCommon_->GetWinApp()->kClientWidth) / float(dxCommon_->GetWinApp()->kClientHeight), 0.1f, 100.0f);
	transformationMatrixUtahTeapot_->Update();
	modelUtahTeapot_->GetMaterial()->Update();
	//stanfordBunny
	transformationMatrixStanfordBunny_->worldMatrix_ = MakeAffineMatrix(transformationMatrixStanfordBunny_->transform_.scale, transformationMatrixStanfordBunny_->transform_.rotate, transformationMatrixStanfordBunny_->transform_.translate);
	transformationMatrixStanfordBunny_->cameraMatrix_ = MakeAffineMatrix(transformationMatrixStanfordBunny_->cameraTransform_.scale, transformationMatrixStanfordBunny_->cameraTransform_.rotate, transformationMatrixStanfordBunny_->cameraTransform_.translate);
	transformationMatrixStanfordBunny_->viewMatrix_ = Inverse(transformationMatrixStanfordBunny_->cameraMatrix_);
	transformationMatrixStanfordBunny_->projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, float(dxCommon_->GetWinApp()->kClientWidth) / float(dxCommon_->GetWinApp()->kClientHeight), 0.1f, 100.0f);
	transformationMatrixStanfordBunny_->Update();
	modelStanfordBunny_->GetMaterial()->Update();

	ImGui::Begin(" ");
	if (ImGui::TreeNode("Triangle")) {
		ImGui::Checkbox("Draw", &drawTriangle_);
		ImGui::DragFloat3("translate", &transformationMatrix_->transform_.translate.x, 0.01f);
		ImGui::DragFloat3("rotate", &transformationMatrix_->transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("scale", &transformationMatrix_->transform_.scale.x, 0.01f);
		ImGui::ColorEdit4("color", &model_->GetMaterial()->color_.x);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Sphere")) {
		ImGui::Checkbox("Draw", &drawSphere_);
		ImGui::DragFloat3("translate", &transformationMatrixSphere_->transform_.translate.x, 0.01f);
		ImGui::DragFloat3("rotate", &transformationMatrixSphere_->transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("scale", &transformationMatrixSphere_->transform_.scale.x, 0.01f);
		ImGui::ColorEdit4("color", &modelSphere_->GetMaterial()->color_.x);
		ImGui::Checkbox("enableLighting", reinterpret_cast<bool*>(&modelSphere_->GetMaterial()->enableLighting_));
		if (ImGui::Button("LambertianReflectance")) {
			modelSphere_->GetMaterial()->lightingMethod_ = LambertianReflectance;
		}
		if (ImGui::Button("HalfLambert")) {
			modelSphere_->GetMaterial()->lightingMethod_ = HalfLambert;
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Sprite")) {
		ImGui::Checkbox("Draw", &drawSprite_);
		ImGui::DragFloat3("translate", &transformationMatrixSprite_->transform_.translate.x, 0.01f);
		ImGui::DragFloat3("rotate", &transformationMatrixSprite_->transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("scale", &transformationMatrixSprite_->transform_.scale.x, 0.01f);
		ImGui::DragFloat3("uvTranslate", &modelSprite_->GetMaterial()->uvTransform_.translate.x, 0.01f);
		ImGui::DragFloat3("uvRotate", &modelSprite_->GetMaterial()->uvTransform_.rotate.x, 0.01f);
		ImGui::DragFloat3("uvScale", &modelSprite_->GetMaterial()->uvTransform_.scale.x, 0.01f);
		ImGui::ColorEdit4("color", &modelSprite_->GetMaterial()->color_.x);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("UtahTeapot")) {
		ImGui::Checkbox("utahTeapot", &drawUtahTeapot_);
		ImGui::DragFloat3("translate", &transformationMatrixUtahTeapot_->transform_.translate.x, 0.01f);
		ImGui::DragFloat3("rotate", &transformationMatrixUtahTeapot_->transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("scale", &transformationMatrixUtahTeapot_->transform_.scale.x, 0.01f);
		ImGui::ColorEdit4("color", &modelUtahTeapot_->GetMaterial()->color_.x);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("StanfordBunny")) {
		ImGui::Checkbox("Draw", &drawStanfordBunny_);
		ImGui::DragFloat3("translate", &transformationMatrixStanfordBunny_->transform_.translate.x, 0.01f);
		ImGui::DragFloat3("rotate", &transformationMatrixStanfordBunny_->transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("scale", &transformationMatrixStanfordBunny_->transform_.scale.x, 0.01f);
		ImGui::ColorEdit4("color", &modelStanfordBunny_->GetMaterial()->color_.x);
		ImGui::TreePop();
	}
	//ImGui::Checkbox("enableLighting", reinterpret_cast<bool*>(&modelSprite_->GetMaterial()->enableLighting_));
	ImGui::End();
};

void GameScene::Draw(GameManager* gameManager) {
	if (drawTriangle_) {
		model_->Draw(transformationMatrix_.get());
	}

	if (drawSphere_) {
		modelSphere_->Draw(transformationMatrixSphere_.get());
	}
	
	if (drawSprite_) {
		modelSprite_->Draw(transformationMatrixSprite_.get());
	}
	
	if (drawUtahTeapot_) {
		modelUtahTeapot_->Draw(transformationMatrixUtahTeapot_.get());
	}
	
	if (drawStanfordBunny_) {
		modelStanfordBunny_->Draw(transformationMatrixStanfordBunny_.get());
	}
};