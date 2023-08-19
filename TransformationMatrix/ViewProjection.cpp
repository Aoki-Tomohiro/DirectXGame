#include "ViewProjection.h"

void ViewProjection::UpdateMatrix() {
	matWorld_ = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, rotation_, translation_);
	matView_ = Inverse(matWorld_);
	matProjection_ = MakePerspectiveFovMatrix(fov_, aspectRatio_, nearClip_, farClip_);
}