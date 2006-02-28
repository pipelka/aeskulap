#include "straight3d.h"

namespace Aeskulap {

RStraight3D Straight3D::create(const RPoint3D& origin, const RVector3D& direction) {
	return RStraight3D(new Straight3D(origin, direction));
}

Straight3D::Straight3D(const RPoint3D& origin, const RVector3D& direction) : 
Point3D(origin),
direction(direction) {
}

void Straight3D::draw(DisplayObject* d) {
}

} // namespace Aeskulap
