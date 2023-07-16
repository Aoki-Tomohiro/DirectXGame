#include <Windows.h>
#include "WinApp.h"
#include "DirectX.h"
#include "Model.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_DX12.h"


//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {

	CoInitializeEx(0, COINIT_MULTITHREADED);

	//WindowsAPI
	WinApp* winApp = new WinApp;
	winApp->CreateGameWindow(L"CG2WindowClass", winApp->kClientWidth, winApp->kClientHeight);

	//DirectX
	DirectXCommon* directX = new DirectXCommon;
	directX->Initialize(winApp);

	//Model
	Model* model = new Model;
	model->Initialize(directX);

	//頂点データ
	ID3D12Resource* vertexResource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	VertexData vertexData[1536];
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
			vertexData[start].position.x = std::cos(lat) * std::cos(lon);
			vertexData[start].position.y = std::sin(lat);
			vertexData[start].position.z = std::cos(lat) * std::sin(lon);
			vertexData[start].position.w = 1.0f;
			vertexData[start].texcoord.x = float(lonIndex) / float(kSubdivision);
			vertexData[start].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			//残りの５頂点も順番に計算して入力していく
			vertexData[start + 1].position.x = std::cos(lat + kLatEvery) * std::cos(lon);
			vertexData[start + 1].position.y = std::sin(lat + kLatEvery);
			vertexData[start + 1].position.z = std::cos(lat + kLatEvery) * std::sin(lon);
			vertexData[start + 1].position.w = 1.0f;
			vertexData[start + 1].texcoord.x = float(lonIndex) / float(kSubdivision);
			vertexData[start + 1].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			vertexData[start + 1].normal.x = vertexData[start + 1].position.x;
			vertexData[start + 1].normal.y = vertexData[start + 1].position.y;
			vertexData[start + 1].normal.z = vertexData[start + 1].position.z;
			vertexData[start + 2].position.x = std::cos(lat) * std::cos(lon + kLonEvery);
			vertexData[start + 2].position.y = std::sin(lat);
			vertexData[start + 2].position.z = std::cos(lat) * std::sin(lon + kLonEvery);
			vertexData[start + 2].position.w = 1.0f;
			vertexData[start + 2].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			vertexData[start + 2].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			vertexData[start + 2].normal.x = vertexData[start + 2].position.x;
			vertexData[start + 2].normal.y = vertexData[start + 2].position.y;
			vertexData[start + 2].normal.z = vertexData[start + 2].position.z;
			vertexData[start + 3].position.x = std::cos(lat) * std::cos(lon + kLonEvery);
			vertexData[start + 3].position.y = std::sin(lat);
			vertexData[start + 3].position.z = std::cos(lat) * std::sin(lon + kLonEvery);
			vertexData[start + 3].position.w = 1.0f;
			vertexData[start + 3].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			vertexData[start + 3].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			vertexData[start + 3].normal.x = vertexData[start + 3].position.x;
			vertexData[start + 3].normal.y = vertexData[start + 3].position.y;
			vertexData[start + 3].normal.z = vertexData[start + 3].position.z;
			vertexData[start + 4].position.x = std::cos(lat + kLatEvery) * std::cos(lon);
			vertexData[start + 4].position.y = std::sin(lat + kLatEvery);
			vertexData[start + 4].position.z = std::cos(lat + kLatEvery) * std::sin(lon);
			vertexData[start + 4].position.w = 1.0f;
			vertexData[start + 4].texcoord.x = float(lonIndex) / float(kSubdivision);
			vertexData[start + 4].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			vertexData[start + 4].normal.x = vertexData[start + 4].position.x;
			vertexData[start + 4].normal.y = vertexData[start + 4].position.y;
			vertexData[start + 4].normal.z = vertexData[start + 4].position.z;
			vertexData[start + 5].position.x = std::cos(lat + kLatEvery) * std::cos(lon + kLonEvery);
			vertexData[start + 5].position.y = std::sin(lat + kLatEvery);
			vertexData[start + 5].position.z = std::cos(lat + kLatEvery) * std::sin(lon + kLonEvery);
			vertexData[start + 5].position.w = 1.0f;
			vertexData[start + 5].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			vertexData[start + 5].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			vertexData[start + 5].normal.x = vertexData[start + 5].position.x;
			vertexData[start + 5].normal.y = vertexData[start + 5].position.y;
			vertexData[start + 5].normal.z = vertexData[start + 5].position.z;
		}
	}
	vertexResource = model->CreateVertexResource(vertexBufferView, sizeof(vertexData), vertexData, 1536);

	//Sprite用の頂点リソース
	ID3D12Resource* vertexResourceSprite = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	VertexData vertexDataSprite[4];
	vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };//左下
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };//左上
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };//右下
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f };//右上
	vertexDataSprite[3].texcoord = { 1.0f,0.0f };
	vertexDataSprite[3].normal = { 0.0f,0.0f, -1.0f };
	vertexResourceSprite = model->CreateVertexResource(vertexBufferViewSprite, sizeof(vertexDataSprite), vertexDataSprite, 4);

	//マテリアルデータ
	ID3D12Resource* materialResource = nullptr;
	Material* materialDataSphere = new Material();
	materialDataSphere->color = { 1.0f,1.0f,1.0f,1.0f };
	materialDataSphere->enableLighting = true;
	materialDataSphere->uvTransform = MakeIdentity4x4();
	materialResource = model->CreateMaterialData(materialDataSphere);
	ID3D12Resource* materialResourceSprite = nullptr;
	Material* materialDataSprite = new Material();
	materialDataSprite->color = { 1.0f,1.0f,1.0f,1.0f };
	materialDataSprite->enableLighting = false;
	materialDataSprite->uvTransform = MakeIdentity4x4();
	materialResourceSprite = model->CreateMaterialData(materialDataSprite);
	Transform uvTransformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};

	//WVP用リソース
	ID3D12Resource* transformationMatrixData = model->CreateBufferResource(directX->GetDevice(), sizeof(TransformationMatrix));
	Transform transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Transform cameraTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
	ID3D12Resource* transformationMatrixResourceSprite = model->CreateBufferResource(directX->GetDevice(), sizeof(TransformationMatrix));
	Transform transformSprite = { { 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f } };
	bool useMonsterBall = true;

	//Lighting
	ID3D12Resource* lightingResource = model->CreateBufferResource(directX->GetDevice(), sizeof(DirectionalLight));
	DirectionalLight* directionalLight = nullptr;
	DirectionalLight lightingData = { {1.0f,1.0f,1.0f,1.0f},{0.0f,-1.0f,0.0f},1.0f };
	lightingResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLight));
	*directionalLight = lightingData;

	//IndexResource
	ID3D12Resource* indexResourceSprite = directX->CreateBufferResource(directX->GetDevice(), sizeof(uint32_t) * 6);
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	//リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;
	//インデックスリソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;

	ID3D12Resource* indexResourceSphere = directX->CreateBufferResource(directX->GetDevice(), sizeof(uint32_t) * 1536);
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSphere{};
	//リソースの先頭のアドレスから使う
	indexBufferViewSphere.BufferLocation = indexResourceSphere->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferViewSphere.SizeInBytes = sizeof(uint32_t) * 1536;
	//インデックスはuint32_tとする
	indexBufferViewSphere.Format = DXGI_FORMAT_R32_UINT;
	//インデックスリソースにデータを書き込む
	uint32_t* indexDataSphere = nullptr;
	indexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSphere));
	for (uint32_t i = 0; i < kSubdivision; ++i) {
		for (uint32_t j = 0; j < kSubdivision; ++j) {
			uint32_t start = (i * kSubdivision + j) * 6;
			indexDataSphere[start] = start;
			indexDataSphere[start + 1] = start + 1;
			indexDataSphere[start + 2] = start + 2;
			indexDataSphere[start + 3] = start + 2;
			indexDataSphere[start + 4] = start + 1;
			indexDataSphere[start + 5] = start + 5;
		}
	}

	//ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_Init(directX->GetDevice(),
		directX->GetSwapChainDesc().BufferCount,
		directX->GetRenderTargetViewDesc().Format,
		directX->GetSRVDescriptorHeap(),
		directX->GetSRVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		directX->GetSRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());

	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		//メインループを抜ける
		if (winApp->MessageRoop() == true) {
			break;
		}

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		//ゲーム処理
		//三角形
		transform.rotate.y += 0.03f;
		Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(winApp->kClientWidth) / float(winApp->kClientHeight), 0.1f, 100.0f);
		TransformationMatrix worldViewProjectionMatrix = { Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix)),worldMatrix };
		model->UpdateMatrix(transformationMatrixData, worldViewProjectionMatrix);

		//sprite
		Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
		Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(winApp->kClientWidth), float(winApp->kClientHeight), 0.0f, 100.0f);
		TransformationMatrix worldViewProjectionMatrixSprite = { Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite)),worldMatrixSprite };
		model->UpdateMatrix(transformationMatrixResourceSprite, worldViewProjectionMatrixSprite);

		//lighting
		lightingResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLight));
		*directionalLight = lightingData;

		//uvTransform
		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
		materialDataSprite->uvTransform = uvTransformMatrix;
		model->UpdateMaterialData(materialResourceSprite, materialDataSprite);

		ImGui::Begin("Window");
		ImGui::DragFloat3("CameraTranslate", &cameraTransform.translate.x, 0.01f);
		ImGui::Checkbox("useMonsterBall", &useMonsterBall);
		ImGui::DragFloat3("spriteTransform", &transformSprite.translate.x, 1.0f);
		ImGui::DragFloat3("cameraScale", &cameraTransform.scale.x, 0.01f);
		ImGui::DragFloat3("cameraRotate", &cameraTransform.rotate.x, 0.01f);
		ImGui::DragFloat3("cameraTranslate", &cameraTransform.translate.x, 0.01f);
		ImGui::DragFloat3("lightDirection", &lightingData.direction.x, 0.01f);
		ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
		ImGui::End();

		//描画始まり
		ImGui::Render();
		directX->PreDraw();

		//オブジェクトの描画
		model->Draw(&vertexBufferView, 1536, materialResource, transformationMatrixData, lightingResource, useMonsterBall, &indexBufferViewSphere);
		model->Draw(&vertexBufferViewSprite, 6, materialResourceSprite, transformationMatrixResourceSprite, lightingResource, false, &indexBufferViewSprite);

		//実際のCommandListのImGuiの描画コマンドを積む
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), directX->GetCommandList());

		//描画終わり
		directX->PostDraw();
	}

	//解放処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	indexResourceSphere->Release();
	indexResourceSprite->Release();
	lightingResource->Release();
	vertexResourceSprite->Release();
	materialResourceSprite->Release();
	transformationMatrixResourceSprite->Release();
	vertexResource->Release();
	materialResource->Release();
	transformationMatrixData->Release();
	delete model;
	delete directX;
	delete winApp;

	CoUninitialize();

	return 0;
}