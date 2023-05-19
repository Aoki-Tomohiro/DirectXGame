#include <Windows.h>
#include "base/WinApp.h"
#include "base/DirectX.h"
#include "Pipeline.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {

	//WindowsAPI
	WinApp* winApp = new WinApp;
	winApp->CreateGameWindow(L"CG2WindowClass", winApp->kClientWidth, winApp->kClientHeight);

	//DirectX
	DirectX* directX = new DirectX;
	directX->Initialize(winApp);

	//Pipeline
	Pipeline* pipeline = new Pipeline;
	pipeline->Initialize(directX);

	//三角形の作成
	ID3D12Resource* resource[3] = { nullptr };
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView[3]{};
	Vector4 pos[3][3] = { 0 };

	resource[0] = pipeline->CreateBufferResource(directX->GetDevice(), sizeof(Vector4) * 3);
	pos[0][0] = { -0.8f,-0.8f,0.0f,1.0f };
	pos[0][1] = { -0.5f,0.0f,0.0f,1.0f };
	pos[0][2] = { -0.2f,-0.8f,0.0f,1.0f };

	resource[1] = pipeline->CreateBufferResource(directX->GetDevice(), sizeof(Vector4) * 3);
	pos[1][0] = { -0.3f,0.0f,0.0f,1.0f };
	pos[1][1] = { 0.0f,0.8f,0.0f,1.0f };
	pos[1][2] = { 0.3f,0.0f,0.0f,1.0f };

	resource[2] = pipeline->CreateBufferResource(directX->GetDevice(), sizeof(Vector4) * 3);
	pos[2][0] = { 0.2f,-0.8f,0.0f,1.0f };
	pos[2][1] = { 0.5f,0.0f,0.0f,1.0f };
	pos[2][2] = { 0.8f,-0.8f,0.0f,1.0f };

	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		//メインループを抜ける
		if (winApp->MessageRoop() == true) {
			break;
		}

		directX->PreDraw();

		pipeline->Draw(resource[0], vertexBufferView[0], pos[0]);
		pipeline->Draw(resource[1], vertexBufferView[1], pos[1]);
		pipeline->Draw(resource[2], vertexBufferView[2], pos[2]);

		directX->PostDraw();
	}

	resource[0]->Release();
	resource[1]->Release();
	resource[2]->Release();
	delete pipeline;
	delete directX;
	delete winApp;

	return 0;
}