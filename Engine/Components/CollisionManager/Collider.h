#pragma once
#include <stdint.h>
#include "MathFunction.h"

class Collider {
public:
	/// <summary>
	/// 衝突時に呼ばれる関数
	/// </summary>
	virtual void OnCollision() {};

	/// <summary>
	/// ワールド座標を取得
	/// </summary>
	/// <returns></returns>
	virtual Vector3 GetWorldPosition() = 0;

	/// <summary>
	/// 半径を取得
	/// </summary>
	/// <returns></returns>
	float GetRadius() { return radius_; };

	/// <summary>
	/// 半径を設定
	/// </summary>
	/// <param name="radius"></param>
	void SetRadius(float radius) { radius_ = radius; };

	/// <summary>
	/// 衝突属性(自分)を取得
	/// </summary>
	/// <returns></returns>
	uint32_t GetCollisionAttribute() { return collisionAttribute_; };

	/// <summary>
	/// 衝突属性(自分)を設定
	/// </summary>
	/// <param name="collisionAttribute"></param>
	void SetCollisionAttribute(uint32_t collisionAttribute) { collisionAttribute_ = collisionAttribute; };

	/// <summary>
	/// 衝突属性(相手)を取得
	/// </summary>
	/// <returns></returns>
	uint32_t GetCollisionMask() { return collisionMask_; };

	/// <summary>
	/// 衝突属性(相手)を設定
	/// </summary>
	/// <param name="collisionMask"></param>
	void SetCollisionMask(uint32_t collisionMask) { collisionMask_ = collisionMask; };

private:
	//衝突半径
	float radius_ = 1.0f;
	//衝突属性(自分)
	uint32_t collisionAttribute_ = 0xffffffff;
	//衝突マスク(相手)
	uint32_t collisionMask_ = 0xffffffff;
};