#pragma once
#include "WinApp.h"
#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800//DirectInputのバージョン指定
#include <dinput.h>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

class Input{
public:
	/// <summary>
	/// シングルインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static Input* GetInstance();

	/// <summary>
	/// シングルトンインスタンスの削除
	/// </summary>
	static void DeleteInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// キーが押されているかを判定
	/// </summary>
	/// <param name="keyNum"></param>
	/// <returns></returns>
	bool PushKey(uint8_t keyNum);

	/// <summary>
	/// キーを離しているかを判定
	/// </summary>
	/// <param name="keyNum"></param>
	/// <returns></returns>
	bool ReleaseKey(uint8_t keyNum);

	/// <summary>
	/// キーを押した瞬間を判定
	/// </summary>
	/// <param name="keyNum"></param>
	/// <returns></returns>
	bool PushKeyEnter(uint8_t keyNum);

	/// <summary>
	/// キーを離した瞬間を判定
	/// </summary>
	/// <param name="keyNum"></param>
	/// <returns></returns>
	bool PushKeyExit(uint8_t keyNum);

	/// <summary>
	/// マウスが押されているかを判定
	/// </summary>
	/// <param name="mouseNum"></param>
	/// <returns></returns>
	bool IsPressMouse(int32_t mouseNum);

	/// <summary>
	/// マウスを離しているかを判定
	/// </summary>
	/// <param name="mouseNum"></param>
	/// <returns></returns>
	bool IsReleaseMouse(int32_t mouseNum);

	/// <summary>
	/// マウスが押された瞬間を判定
	/// </summary>
	/// <param name="mouseNum"></param>
	/// <returns></returns>
	bool IsPressMouseEnter(int32_t mouseNum);

	/// <summary>
	/// マウスが離された瞬間を判定
	/// </summary>
	/// <param name="mouseNum"></param>
	/// <returns></returns>
	bool IsPressMouseExit(int32_t mouseNum);

	/// <summary>
	/// ホイールスクロール量を取得する
	/// </summary>
	/// <returns></returns>
	int32_t GetWheel();

private:
	static Input* instance;
	Microsoft::WRL::ComPtr<IDirectInput8> directInput_ = nullptr;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboardDevice_ = nullptr;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> mouseDevice_ = nullptr;
	BYTE key_[256] = {};
	BYTE preKey_[256] = {};
	DIMOUSESTATE mouse_ = {};
	DIMOUSESTATE mousePre_ = {};
};

