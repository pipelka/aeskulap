#ifndef AESKULAP_STRAIGHT2D_H
#define AESKULAP_STRAIGHT2D_H

#include "point2d.h"

namespace Aeskulap {

class Straight2D;
typedef Glib::RefPtr<Aeskulap::Straight2D> RStraight2D;

class Straight2D : public Point2D {
public:

	void draw(DisplayObject* d);

	static RStraight2D create(
			const RPoint2D& origin,
			const RVector2D& direction);

	RPoint2D intersect(const RStraight2D& b);

	RVector2D direction;
	
protected:

	Straight2D(const RPoint2D& origin, const RVector2D& direction);

};

} // namespace Aeskulap

#endif // AESKULAP_STRAIGHT2D_H
