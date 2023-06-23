#include <Windows.h>
#include "WinApp.h"
#include "DirectX.h"
#include "Model.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_DX12.h"


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
	vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };
	vertexData[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[3].position = { -0.5f,-0.5f,0.5f,1.0f };
	vertexData[3].texcoord = { 0.0f,1.0f };
	vertexData[4].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[4].texcoord = { 0.5f,0.0f };
	vertexData[5].position = { 0.5f,-0.5f,-0.5f,1.0f };
	vertexData[5].texcoord = { 1.0f,1.0f };
	vertexResource = model->CreateBufferResource(directX->GetDevice(), sizeof(VertexData) * 1536);
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
			//残りの５頂点も順番に計算して入力していく
			vertexData[start + 1].position.x = std::cos(lat + kLatEvery) * std::cos(lon);
			vertexData[start + 1].position.y = std::sin(lat + kLatEvery);
			vertexData[start + 1].position.z = std::cos(lat + kLatEvery) * std::sin(lon);
			vertexData[start + 1].position.w = 1.0f;
			vertexData[start + 1].texcoord.x = float(lonIndex) / float(kSubdivision); 
			vertexData[start + 1].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			vertexData[start + 2].position.x = std::cos(lat) * std::cos(lon + kLonEvery);
			vertexData[start + 2].position.y = std::sin(lat);
			vertexData[start + 2].position.z = std::cos(lat) * std::sin(lon + kLonEvery);
			vertexData[start + 2].position.w = 1.0f;
			vertexData[start + 2].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			vertexData[start + 2].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			vertexData[start + 3].position.x = std::cos(lat) * std::cos(lon + kLonEvery);
			vertexData[start + 3].position.y = std::sin(lat);
			vertexData[start + 3].position.z = std::cos(lat) * std::sin(lon + kLonEvery);
			vertexData[start + 3].position.w = 1.0f;
			vertexData[start + 3].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			vertexData[start + 3].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			vertexData[start + 4].position.x = std::cos(lat + kLatEvery) * std::cos(lon);
			vertexData[start + 4].position.y = std::sin(lat + kLatEvery);
			vertexData[start + 4].position.z = std::cos(lat + kLatEvery) * std::sin(lon);
			vertexData[start + 4].position.w = 1.0f;
			vertexData[start + 4].texcoord.x = float(lonIndex) / float(kSubdivision);
			vertexData[start + 4].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			vertexData[start + 5].position.x = std::cos(lat + kLatEvery) * std::cos(lon + kLonEvery);
			vertexData[start + 5].position.y = std::sin(lat + kLatEvery);
			vertexData[start + 5].position.z = std::cos(lat + kLatEvery) * std::sin(lon + kLonEvery);
			vertexData[start + 5].position.w = 1.0f;
			vertexData[start + 5].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			vertexData[start + 5].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
		}
	}


	//Sprite用の頂点リソース
	ID3D12Resource* vertexResourceSprite = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	VertexData vertexDataSprite[6];
	//一枚目
	vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };//左下
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };//左上
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };//右下
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	//二枚目
	vertexDataSprite[3].position = { 0.0f,0.0f,0.0f,1.0f };//左上
	vertexDataSprite[3].texcoord = { 0.0f,0.0f };
	vertexDataSprite[4].position = { 640.0f,0.0f,0.0f,1.0f };//右上
	vertexDataSprite[4].texcoord = { 1.0f,0.0f };
	vertexDataSprite[5].position = { 640.0f,360.0f,0.0f,1.0f };//右下
	vertexDataSprite[5].texcoord = { 1.0f,1.0f };
	vertexResourceSprite = model->CreateBufferResource(directX->GetDevice(), sizeof(vertexDataSprite));

	//マテリアルデータ
	ID3D12Resource* materialResource = nullptr;
	Vector4 color = { 1.0f,1.0f,1.0f,1.0f };
	materialResource = model->CreateBufferResource(directX->GetDevice(), sizeof(color));
	ID3D12Resource* materialResourceSprite = nullptr;
	Vector4 spriteColor = { 1.0f,1.0f,1.0f,1.0f };
	materialResourceSprite = model->CreateBufferResource(directX->GetDevice(), sizeof(spriteColor));

	//WVP用リソース
	ID3D12Resource* transformationMatrixData = nullptr;
	Transform transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Transform cameraTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
	transformationMatrixData = model->CreateBufferResource(directX->GetDevice(), sizeof(Matrix4x4));
	
	ID3D12Resource* transformationMatrixResourceSprite = nullptr;
	Transform transformSprite = { { 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f } };
	transformationMatrixResourceSprite = model->CreateBufferResource(directX->GetDevice(), sizeof(Matrix4x4));

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
		ImGui::ShowDemoWindow();
		//ゲーム処理
		//三角形
		/*transform.rotate.y += 0.03f;*/
		Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(winApp->kClientWidth) / float(winApp->kClientHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		model->UpdateMatrix(transformationMatrixData, worldViewProjectionMatrix);

		//sprite
		Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
		Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(winApp->kClientWidth), float(winApp->kClientHeight), 0.0f, 100.0f);
		Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
		model->UpdateMatrix(transformationMatrixResourceSprite, worldViewProjectionMatrixSprite);
		ImGui::Begin("Window");
		ImGui::DragFloat3("spriteTransform", &transformSprite.translate.x, 1.0f);
		ImGui::DragFloat3("cameraScale", &cameraTransform.scale.x, 0.01f);
		ImGui::DragFloat3("cameraRotate", &cameraTransform.rotate.x, 0.01f);
		ImGui::DragFloat3("cameraTranslate", &cameraTransform.translate.x, 0.01f);
		ImGui::End();

		//描画始まり
		ImGui::Render();
		directX->PreDraw();

		//オブジェクトの描画
		model->Draw(vertexResource, vertexBufferView, vertexData, sizeof(vertexData), 1536, materialResource, &color, transformationMatrixData);
		model->Draw(vertexResourceSprite, vertexBufferViewSprite, vertexDataSprite, sizeof(vertexDataSprite), 6, materialResourceSprite, &spriteColor, transformationMatrixResourceSprite);

		//実際のCommandListのImGuiの描画コマンドを積む
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), directX->GetCommandList());

		//描画終わり
		directX->PostDraw();
	}

	//解放処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
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