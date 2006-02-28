#include "plane3d.h"

namespace Aeskulap {

RPlane3D Plane3D::create(const RPoint3D& origin, const RVector3D& direction_x, const RVector3D& direction_y) {
	return RPlane3D(new Plane3D(origin, direction_x, direction_y));
}

void Plane3D::draw(DisplayObject* d) {
}

Plane3D::Plane3D(const RPoint3D& origin, const RVector3D& direction_x, const RVector3D& direction_y) :
Point3D(origin),
m_direction_x(direction_x),
m_direction_y(direction_y) {
}

RPoint2D Plane3D::project_parallel(const RPoint3D& p) {
	RPoint3D b = p - RPoint3D(this);
	return Point2D::create(
			m_direction_x->x * b->x + m_direction_x->y * b->y + m_direction_x->z * b->z,
			m_direction_y->x * b->x + m_direction_y->y * b->y + m_direction_y->z * b->z);
}

RLine2D Plane3D::project_parallel(const RLine3D& p) {
	RPoint2D a = project_parallel(p->start());
	RPoint2D b = project_parallel(p->end());
	
	return Line2D::create(a, b);
}

RPoint3D Plane3D::transform_to_world(const RPoint2D& p) {
	return RPoint3D(this) + m_direction_x * p->x + m_direction_y * p->y;
}

} // namespace Aeskulap
