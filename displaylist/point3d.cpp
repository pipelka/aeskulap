#include "point3d.h"
#include "adisplayobject.h"

namespace Aeskulap {

RPoint3D Point3D::create(double px, double py, double pz) {
	return RPoint3D(new Point3D(px, py, pz));
}

Point3D::Point3D(double px, double py, double pz) : Point2D(px, py), z(pz) {
}

Point3D::Point3D(const RPoint3D& r) : Point2D(r->x, r->y) {
	z = r->z;
}

void Point3D::draw(DisplayObject* d) {
	RPoint2D p = d->get_plane()->project_parallel(RPoint3D(this));
	p->draw(d);
}


RPoint3D operator+(const RPoint3D& a, const RPoint3D& b) {
	return Point3D::create(
			a->x + b->x,
			a->y + b->y,
			a->z + b->z
			);
}

RPoint3D operator-(const RPoint3D& a, const RPoint3D& b) {
	return Point3D::create(
			a->x - b->x,
			a->y - b->y,
			a->z - b->z
			);
}

RPoint3D operator*(const RPoint3D& a, double f) {
	return Point3D::create(
			a->x * f,
			a->y * f,
			a->z * f
			);
}

RPoint3D operator/(const RPoint3D& a, double f) {
	return Point3D::create(
			a->x / f,
			a->y / f,
			a->z / f
			);
}

} // namespace Aeskulap
