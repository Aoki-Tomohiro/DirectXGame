#pragma once
#include <cmath>
#include <vector>
#include <string>

struct Vector2 {
	float x;
	float y;
};

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Vector4 {
	float x;
	float y;
	float z;
	float w;
};

struct Matrix3x3 {
	float m[3][3];
};

struct Matrix4x4 {
	float m[4][4];
};

//struct Transform {
//	Vector3 scale;
//	Vector3 rotate;
//	Vector3 translate;
//};

/// <summary>
/// ベクトルの足し算
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <returns></returns>
Vector3 Add(const Vector3& v1, const Vector3& v2);

/// <summary>
/// ベクトルの引き算
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <returns></returns>
Vector3 Subtract(const Vector3& v1, const Vector3& v2);

/// <summary>
/// ベクトルの掛け算
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <returns></returns>
Vector3 Multiply(const Vector3& v1, const Vector3& v2);

/// <summary>
/// ベクトルの掛け算
/// </summary>
/// <param name="v"></param>
/// <param name="speed"></param>
/// <returns></returns>
Vector3 Multiply(const Vector3& v, const float speed);

/// <summary>
/// 内積
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <returns></returns>
float Dot(const Vector3& v1, const Vector3& v2);

/// <summary>
/// 長さ
/// </summary>
/// <param name="v"></param>
/// <returns></returns>
float Length(const Vector3& v);

/// <summary>
/// 正規化
/// </summary>
/// <param name="v"></param>
/// <returns></returns>
Vector3 Normalize(const Vector3& v);

/// <summary>
/// 行列の掛け算
/// </summary>
/// <param name="m1"></param>
/// <param name="m2"></param>
/// <returns></returns>
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

/// <summary>
/// 逆行列
/// </summary>
/// <param name="m"></param>
/// <returns></returns>
Matrix4x4 Inverse(const Matrix4x4& m);

/// <summary>
/// 転置行列
/// </summary>
/// <param name="m"></param>
/// <returns></returns>
Matrix4x4 Transpose(const Matrix4x4& m);

/// <summary>
/// 単位行列
/// </summary>
/// <returns></returns>
Matrix4x4 MakeIdentity4x4();

/// <summary>
/// 平行移動行列
/// </summary>
/// <param name="translate"></param>
/// <returns></returns>
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

/// <summary>
/// スケール行列
/// </summary>
/// <param name="scale"></param>
/// <returns></returns>
Matrix4x4 MakeScaleMatrix(const Vector3& scale);

/// <summary>
/// X軸回転行列
/// </summary>
/// <param name="radian"></param>
/// <returns></returns>
Matrix4x4 MakeRotateXMatrix(float radian);

/// <summary>
/// Y軸回転行列
/// </summary>
/// <param name="radian"></param>
/// <returns></returns>
Matrix4x4 MakeRotateYMatrix(float radian);

/// <summary>
/// Z軸回転行列
/// </summary>
/// <param name="radian"></param>
/// <returns></returns>
Matrix4x4 MakeRotateZMatrix(float radian);

/// <summary>
/// アフィン行列
/// </summary>
/// <param name="scale"></param>
/// <param name="rotate"></param>
/// <param name="translate"></param>
/// <returns></returns>
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

/// <summary>
/// 透視投影行列
/// </summary>
/// <param name="fovY"></param>
/// <param name="aspectRatio"></param>
/// <param name="nearClip"></param>
/// <param name="farClip"></param>
/// <returns></returns>
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

/// <summary>
/// 平行投影行列
/// </summary>
/// <param name="left"></param>
/// <param name="top"></param>
/// <param name="right"></param>
/// <param name="bottom"></param>
/// <param name="nearClip"></param>
/// <param name="farClip"></param>
/// <returns></returns>
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

/// <summary>
/// ビューポート行列
/// </summary>
/// <param name="left"></param>
/// <param name="top"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="minDepth"></param>
/// <param name="maxDepth"></param>
/// <returns></returns>
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

/// <summary>
/// 座標変換
/// </summary>
/// <param name="v"></param>
/// <param name="m"></param>
/// <returns></returns>
Vector3 Transform(const Vector3& v, const Matrix4x4& m);

/// <summary>
/// ベクトル変換
/// </summary>
/// <param name="v"></param>
/// <param name="m"></param>
/// <returns></returns>
Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

/// <summary>
/// 線形補間
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <param name="t"></param>
/// <returns></returns>
float Lerp(const float& v1, const float& v2, float t);

/// <summary>
/// 線形補間
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <param name="t"></param>
/// <returns></returns>
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

/// <summary>
/// 球面線形補間
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <param name="t"></param>
/// <returns></returns>
Vector3 Slerp(const Vector3& v1, const Vector3& v2, float t);

/// <summary>
/// 最短角度補間
/// </summary>
/// <param name="a"></param>
/// <param name="b"></param>
/// <param name="t"></param>
/// <returns></returns>
float LerpShortAngle(const float& a, const float& b, float t);