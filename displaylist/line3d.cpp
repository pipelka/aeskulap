#include "line3d.h"
#include "line2d.h"
#include "adisplayobject.h"

namespace Aeskulap {

void Line3D::draw(DisplayObject* d) {
	RLine2D l = d->get_plane()->project_parallel(RLine3D(this));
	l->draw(d);
}

RLine3D Line3D::create(const RPoint3D& start, const RPoint3D& end) {
	return RLine3D(new Line3D(start, end));
}

Line3D::Line3D(const RPoint3D& a, const RPoint3D& b) : 
Straight3D(a, a - b),
m_end(b) {
}

RPoint3D Line3D::start() {
	return RPoint3D(this);
}

const RPoint3D& Line3D::end() {
	return m_end;
}

} // namespace Aeskulap
