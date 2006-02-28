#include "straight2d.h"

namespace Aeskulap {

RStraight2D Straight2D::create(const RPoint2D& origin, const RVector2D& direction) {
	return RStraight2D(new Straight2D(origin, direction));
}

Straight2D::Straight2D(const RPoint2D& origin, const RVector2D& direction) : 
Point2D(origin),
direction(direction) {
}

void Straight2D::draw(DisplayObject* d) {
}

RPoint2D Straight2D::intersect(const RStraight2D& b) {
	try {
		double t1 = b->direction->y*(x - b->x) - b->direction->x*(y - b->y);
		double t2 = direction->y*b->direction->x - b->direction->y*direction->x;
		
		return RPoint2D(this) + direction * (t1 / t2);
	}
	catch(...) {
		return RPoint2D();
	}
}

} // namespace Aeskulap
