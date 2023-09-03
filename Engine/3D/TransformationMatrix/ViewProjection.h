#pragma once
#include "Math/MathFunction.h"

struct ViewProjection {
	//回転
	Vector3 rotation_ = { 0.0f,0.0f,0.0f };
	//座標
	Vector3 translation_ = { 0.0f,0.0f,-50.0f };
	//ビュー行列
	Matrix4x4 matView_{};
	//プロジェクション行列
	Matrix4x4 matProjection_{};
	//視野角
	float fov_ = 0.45f;
	//アスペクト比
	float aspectRatio_ = 1280.0f / 720.0f;
	//Near
	float nearClip_ = 0.1f;
	//Far
	float farClip_ = 100.0f;

	/// <summary>
	/// 行列の計算
	/// </summary>
	void UpdateMatrix();
};