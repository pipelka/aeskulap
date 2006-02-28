#ifndef AESKULAP_STRAIGHT3D_H
#define AESKULAP_STRAIGHT3D_H

#include "point3d.h"

namespace Aeskulap {

class Straight3D;
typedef Glib::RefPtr<Aeskulap::Straight3D> RStraight3D;

class Straight3D : public Point3D {
public:

	void draw(DisplayObject* d);

	static RStraight3D create(
			const RPoint3D& origin,
			const RVector3D& direction);

	RVector3D direction;
	
protected:

	Straight3D(const RPoint3D& origin, const RVector3D& direction);

};

} // namespace Aeskulap

#endif // AESKULAP_STRAIGHT3D_H
