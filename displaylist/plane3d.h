#ifndef AESKULAP_PLANE3D_H
#define AESKULAP_PLANE3D_H

#include "point3d.h"
#include "line2d.h"
#include "line3d.h"

namespace Aeskulap {

class Plane3D;
typedef Glib::RefPtr<Aeskulap::Plane3D> RPlane3D;

class Plane3D : public Point3D {
public:

	void draw(DisplayObject* d);

	RPoint2D project_parallel(const RPoint3D& p);

	RLine2D project_parallel(const RLine3D& p);

	RPoint3D transform_to_world(const RPoint2D& p);

	static RPlane3D create(
			const RPoint3D& origin = Point3D::create(0, 0, 0),
			const RVector3D& direction_x = Vector3D::create(1, 0, 0),
			const RVector3D& direction_y = Vector3D::create(0, 1, 0));

protected:

	Plane3D(const RPoint3D& origin, const RVector3D& direction_x, const RVector3D& direction_y);

	RVector3D m_direction_x;

	RVector3D m_direction_y;
	
};

} // namespace Aeskulap

#endif // AESKULAP_PLANE3D_H
