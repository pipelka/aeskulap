#ifndef AESKULAP_LINE3D_H
#define AESKULAP_LINE3D_H

#include "straight3d.h"

namespace Aeskulap {

class Line3D : public Straight3D {
public:

	void draw(DisplayObject* d);

	static Glib::RefPtr<Aeskulap::Line3D> create(
			const RPoint3D& start,
			const RPoint3D& end);

	RPoint3D start();

	const RPoint3D& end();
	
protected:

	Line3D(const RPoint3D& a, const RPoint3D& b);

	RPoint3D m_end;

};

typedef Glib::RefPtr<Aeskulap::Line3D> RLine3D;

} // namespace Aeskulap

#endif // AESKULAP_LINE3D_H
