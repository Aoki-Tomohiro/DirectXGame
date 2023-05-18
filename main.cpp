#include <Windows.h>
#include "base/WinApp.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {

	WinApp* winApp = new WinApp;
	winApp->CreateGameWindow(L"CG2WindowClass", winApp->kClientWidth, winApp->kClientHeight);

	//ウィンドウの×ボタンが押されるまでループ
	while (true) {

		//メインループを抜ける
		if (winApp->MessageRoop() == true) {
			break;
		}
	}

	return 0;
}