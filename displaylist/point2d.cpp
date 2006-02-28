#include "point2d.h"
#include "adisplayobject.h"
#include <cmath>

namespace Aeskulap {

RPoint2D Point2D::create(double px, double py) {
	return RPoint2D(new Point2D(px, py));
}

Point2D::Point2D(double px, double py) : x(px), y(py) {
}

Point2D::Point2D(const RPoint2D& r) {
	x = r->x;
	y = r->y;
}

void Point2D::draw(DisplayObject* d) {
	int x;
	int y;

	if(!d->point_to_screen(*this, x, y)) {
		return;
	}
	
	d->draw_point(x, y);
}

double Point2D::length() {
	return sqrt(x*x + y*y);
}
	
RPoint2D Point2D::normalize() {
	return Point2D::create(
		x / length(),
		y / length());
}

RPoint2D operator+(const RPoint2D& a, const RPoint2D& b) {
	return Point2D::create(
			a->x + b->x,
			a->y + b->y);
}

RPoint2D operator-(const RPoint2D& a, const RPoint2D& b) {
	return Point2D::create(
			a->x - b->x,
			a->y - b->y);
}

RPoint2D operator*(const RPoint2D& a, double f) {
	return Point2D::create(
			a->x * f,
			a->y * f);
}

RPoint2D operator/(const RPoint2D& a, double f) {
	return Point2D::create(
			a->x / f,
			a->y / f);
}

} // namespace Aeskulap
