#ifndef AESKULAP_POINT2D_H
#define AESKULAP_POINT2D_H

#include <glibmm.h>
#include "aobject.h"

namespace Aeskulap {

class DisplayObject;

class Point2D;
typedef Glib::RefPtr<Aeskulap::Point2D> RPoint2D;

class Point2D : public Object {
public:

	virtual void draw(DisplayObject* d);

	static RPoint2D create(double px = 0, double py = 0);

	double length();
	
	RPoint2D normalize();

	double x;

	double y;

protected:

	Point2D(double px, double py);

	Point2D(const RPoint2D& r);

};

typedef Point2D Vector2D;
typedef Glib::RefPtr<Aeskulap::Vector2D> RVector2D;

RPoint2D operator+(const RPoint2D& a, const RPoint2D& b);

RPoint2D operator-(const RPoint2D& a, const RPoint2D& b);

RPoint2D operator*(const RPoint2D& a, double f);

RPoint2D operator/(const RPoint2D& a, double f);

} // namespace Aeskulap



#endif // AESKULAP_POINT2D_H
