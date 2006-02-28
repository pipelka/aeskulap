#ifndef AESKULAP_POINT3D_H
#define AESKULAP_POINT3D_H

#include "aobject.h"
#include "point2d.h"

namespace Aeskulap {

class DisplayObject;

class Point3D;
typedef Glib::RefPtr<Aeskulap::Point3D> RPoint3D;

class Point3D : public Point2D {
public:

	virtual void draw(DisplayObject* d);

	double z;

	static RPoint3D create(double px = 0, double py = 0, double pz = 0);

protected:

	Point3D(double px = 0, double py = 0, double pz = 0);

	Point3D(const RPoint3D& r);

};

typedef Point3D Vector3D;
typedef Glib::RefPtr<Aeskulap::Vector3D> RVector3D;

RPoint3D operator+(const RPoint3D& a, const RPoint3D& b);

RPoint3D operator-(const RPoint3D& a, const RPoint3D& b);

RPoint3D operator*(const RPoint3D& a, double f);

RPoint3D operator/(const RPoint3D& a, double f);

} // namespace Aeskulap

#endif // AESKULAP_POINT2D_H
