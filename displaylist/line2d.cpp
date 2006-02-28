#include "line2d.h"
#include "adisplayobject.h"

namespace Aeskulap {

void Line2D::draw(DisplayObject* d) {
	int x0, y0;
	int x1, y1;
	
	if(!d->point_to_screen(*this, x0, y0)) {
		return;
	}

	if(!d->point_to_screen(m_end, x1, y1)) {
		return;
	}

	d->draw_line(x0, y0, x1, y1);
}

RLine2D Line2D::create(const RPoint2D& start, const RPoint2D& end) {
	return RLine2D(new Line2D(start, end));
}

Line2D::Line2D(const RPoint2D& a, const RPoint2D& b) : 
Straight2D(a, a - b),
m_end(b) {
}

RPoint2D Line2D::start() {
	return RPoint2D(this);
}

const RPoint2D& Line2D::end() {
	return m_end;
}


} // namespace Aeskulap
