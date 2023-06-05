#include <Windows.h>
#include "base/WinApp.h"
#include "base/DirectX.h"
#include "Model.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_DX12.h>


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
	ID3D12Resource* vertexResource[3] = { nullptr };
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView[3]{};
	VertexData vertexData[3][3] = { 0 };
	vertexResource[0] = model->CreateBufferResource(directX->GetDevice(), sizeof(vertexData) * 3);
	vertexData[0][0].position = { -0.8f,-0.8f,0.0f,1.0f };
	vertexData[0][0].texcoord = { 0.0f,1.0f };
	vertexData[0][1].position = { -0.5f,0.0f,0.0f,1.0f };
	vertexData[0][1].texcoord = { 0.5f,0.0f };
	vertexData[0][2].position = { -0.2f,-0.8f,0.0f,1.0f };
	vertexData[0][2].texcoord = { 1.0f,1.0f };
	vertexResource[1] = model->CreateBufferResource(directX->GetDevice(), sizeof(vertexData) * 3);
	vertexData[1][0].position = { -0.3f,0.0f,0.0f,1.0f };
	vertexData[1][0].texcoord = { 0.0f,1.0f };
	vertexData[1][1].position = { 0.0f,0.8f,0.0f,1.0f };
	vertexData[1][1].texcoord = { 0.5f,0.0f };
	vertexData[1][2].position = { 0.3f,0.0f,0.0f,1.0f };
	vertexData[1][2].texcoord = { 1.0f,1.0f };
	vertexResource[2] = model->CreateBufferResource(directX->GetDevice(), sizeof(vertexData) * 3);
	vertexData[2][0].position = { 0.2f,-0.8f,0.0f,1.0f };
	vertexData[2][0].texcoord = { 0.0f,1.0f };
	vertexData[2][1].position = { 0.5f,0.0f,0.0f,1.0f };
	vertexData[2][1].texcoord = { 0.5f,0.0f };
	vertexData[2][2].position = { 0.8f,-0.8f,0.0f,1.0f };
	vertexData[2][2].texcoord = { 1.0f,1.0f };

	//マテリアルデータ
	ID3D12Resource* materialResource[3] = { nullptr };
	Vector4 color[3] = { {1.0f,1.0f,1.0f,1.0f},{1.0f,1.0f,1.0f,1.0f},{1.0f,1.0f,1.0f,1.0f} };
	materialResource[0] = model->CreateBufferResource(directX->GetDevice(), sizeof(Vector4));
	materialResource[1] = model->CreateBufferResource(directX->GetDevice(), sizeof(Vector4));
	materialResource[2] = model->CreateBufferResource(directX->GetDevice(), sizeof(Vector4));

	//WVP用リソース
	ID3D12Resource* WVPResource[3] = { nullptr };
	Transform transform[3] = {
		{{1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}},
		{{1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}},
		{{1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}},
	};
	Matrix4x4 worldMatrix[3];
	for (int i = 0; i < 3; i++) {
		WVPResource[i] = model->CreateBufferResource(directX->GetDevice(), sizeof(Matrix4x4));
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
		ImGui::ShowDemoWindow();
		//ゲーム処理
		transform[0].rotate.y += 0.03f;
		worldMatrix[0] = MakeAffineMatrix(transform[0].scale, transform[0].rotate, transform[0].translate);
		transform[1].rotate.y += 0.03f;
		worldMatrix[1] = MakeAffineMatrix(transform[1].scale, transform[1].rotate, transform[1].translate);
		transform[2].rotate.y += 0.03f;
		worldMatrix[2] = MakeAffineMatrix(transform[2].scale, transform[2].rotate, transform[2].translate);
		model->UpdateMatrix(WVPResource[0], worldMatrix[0]);
		model->UpdateMatrix(WVPResource[1], worldMatrix[1]);
		model->UpdateMatrix(WVPResource[2], worldMatrix[2]);

		//描画始まり
		ImGui::Render();
		directX->PreDraw();

		//オブジェクトの描画
		model->Draw(vertexResource[0], vertexBufferView[0], vertexData[0], materialResource[0], &color[0], WVPResource[0]);
		model->Draw(vertexResource[1], vertexBufferView[1], vertexData[1], materialResource[1], &color[1], WVPResource[1]);
		model->Draw(vertexResource[2], vertexBufferView[2], vertexData[2], materialResource[2], &color[2], WVPResource[2]);

		//実際のCommandListのImGuiの描画コマンドを積む
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), directX->GetCommandList());

		//描画終わり
		directX->PostDraw();
	}

	//解放処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	for (int i = 0; i < 3; i++) {
		vertexResource[i]->Release();
		materialResource[i]->Release();
		WVPResource[i]->Release();
	}
	delete model;
	delete directX;
	delete winApp;

	CoUninitialize();

	return 0;
}