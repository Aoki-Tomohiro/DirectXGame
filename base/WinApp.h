#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>

class WinApp {
public:
	//ウィンドウサイズ
	static const int kClientWidth = 1280;
	static const int kClientHeight = 720;

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static WinApp* GetInstance();

	/// <summary>
	/// シングルトンインスタンスの削除
	/// </summary>
	static void DeleteInstance();

	/// <summary>
	/// ウィンドウプロシージャ
	/// </summary>
	/// <param name="hwnd"></param>
	/// <param name="msg"></param>
	/// <param name="wparam"></param>
	/// <param name="lparam"></param>
	/// <returns></returns>
	static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	/// <summary>
	/// ログ
	/// </summary>
	/// <param name="message"></param>
	void Log(const std::string& message);

	/// <summary>
	/// stringをwstringに変換
	/// </summary>
	/// <param name="str"></param>
	/// <returns></returns>
	std::wstring ConvertString(const std::string& str);

	/// <summary>
	/// wstringをstringに変換
	/// </summary>
	/// <param name="str"></param>
	/// <returns></returns>
	std::string ConvertString(const std::wstring& str);

	/// <summary>
	/// ゲームウィンドウの作成
	/// </summary>
	/// <param name="title"></param>
	/// <param name="kClientWidth"></param>
	/// <param name="kClientHeight"></param>
	void CreateGameWindow(const wchar_t* title, int32_t kClientWidth, int32_t kClientHeight);

	/// <summary>
	/// ゲームウィンドウを閉じる
	/// </summary>
	void CloseGameWindow();

	/// <summary>
	/// メッセージの処理
	/// </summary>
	/// <returns></returns>
	bool MessageRoop();

	/// <summary>
	/// ウィンドウハンドルの取得
	/// </summary>
	/// <returns></returns>
	HWND GetHwnd() { return hwnd_; };
private:
	WinApp() = default;
	~WinApp() = default;
	WinApp(const WinApp&) = delete;
	const WinApp& operator=(const WinApp&) = delete;
private:
	static WinApp* instance;
	WNDCLASS wc_{};
	RECT wrc_{};
	HWND hwnd_{};
	MSG msg_{};
};