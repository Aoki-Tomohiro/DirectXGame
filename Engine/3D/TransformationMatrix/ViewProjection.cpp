#include "ViewProjection.h"

void ViewProjection::UpdateMatrix() {
	Matrix4x4 worldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, rotation_, translation_);
	matView_ = Inverse(worldMatrix);
	matProjection_ = MakePerspectiveFovMatrix(fov_, aspectRatio_, nearClip_, farClip_);
}