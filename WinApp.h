#pragma once
#include <Windows.h>
#include <cstdint>

class WinApp {
public:
	~WinApp();
	static const int kClientWidth = 1280;
	static const int kClientHeight = 720;
	static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	void CreateGameWindow(const wchar_t* title, int32_t kClientWidth, int32_t kClientHeight);
	bool MessageRoop();
	HWND GetHwnd() { return hwnd_; };
private:
	WNDCLASS wc_{};
	RECT wrc_{};
	HWND hwnd_{};
	MSG msg_{};
};