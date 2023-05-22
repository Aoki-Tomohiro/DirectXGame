#include <Windows.h>
#include "base/WinApp.h"
#include "base/DirectX.h"
#include "Model.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {

	//WindowsAPI
	WinApp* winApp = new WinApp;
	winApp->CreateGameWindow(L"CG2WindowClass", winApp->kClientWidth, winApp->kClientHeight);

	//DirectX
	DirectX* directX = new DirectX;
	directX->Initialize(winApp);

	//Model
	Model* model = new Model;
	model->Initialize(directX);

	//頂点データ
	ID3D12Resource* vertexResource[3] = { nullptr };
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView[3]{};
	Vector4 pos[3][3] = { 0 };
	vertexResource[0] = model->CreateBufferResource(directX->GetDevice(), sizeof(Vector4) * 3);
	pos[0][0] = { -0.8f,-0.8f,0.0f,1.0f };
	pos[0][1] = { -0.5f,0.0f,0.0f,1.0f };
	pos[0][2] = { -0.2f,-0.8f,0.0f,1.0f };
	vertexResource[1] = model->CreateBufferResource(directX->GetDevice(), sizeof(Vector4) * 3);
	pos[1][0] = { -0.3f,0.0f,0.0f,1.0f };
	pos[1][1] = { 0.0f,0.8f,0.0f,1.0f };
	pos[1][2] = { 0.3f,0.0f,0.0f,1.0f };
	vertexResource[2] = model->CreateBufferResource(directX->GetDevice(), sizeof(Vector4) * 3);
	pos[2][0] = { 0.2f,-0.8f,0.0f,1.0f };
	pos[2][1] = { 0.5f,0.0f,0.0f,1.0f };
	pos[2][2] = { 0.8f,-0.8f,0.0f,1.0f };

	//マテリアルデータ
	ID3D12Resource* materialResource[3] = { nullptr };
	Vector4 color[3] = { {1.0f,0.0f,0.0f,1.0f},{1.0f,0.0f,0.0f,1.0f},{1.0f,0.0f,0.0f,1.0f} };
	materialResource[0] = model->CreateBufferResource(directX->GetDevice(), sizeof(Vector4));
	materialResource[1] = model->CreateBufferResource(directX->GetDevice(), sizeof(Vector4));
	materialResource[2] = model->CreateBufferResource(directX->GetDevice(), sizeof(Vector4));

	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		//メインループを抜ける
		if (winApp->MessageRoop() == true) {
			break;
		}

		//描画始まり
		directX->PreDraw();

		//オブジェクトの描画
		model->Draw(vertexResource[0], vertexBufferView[0], pos[0], materialResource[0], &color[0]);
		model->Draw(vertexResource[1], vertexBufferView[1], pos[1], materialResource[1], &color[1]);
		model->Draw(vertexResource[2], vertexBufferView[2], pos[2], materialResource[2], &color[2]);

		//描画終わり
		directX->PostDraw();
	}

	//解放処理
	for (int i = 0; i < 3; i++) {
		vertexResource[i]->Release();
		materialResource[i]->Release();
	}
	delete model;
	delete directX;
	delete winApp;

	return 0;
}