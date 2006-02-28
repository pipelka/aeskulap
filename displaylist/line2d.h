#ifndef AESKULAP_LINE2D_H
#define AESKULAP_LINE2D_H

#include "straight2d.h"

namespace Aeskulap {

class Line2D : public Straight2D {
public:

	void draw(DisplayObject* d);

	static Glib::RefPtr<Aeskulap::Line2D> create(
			const RPoint2D& start,
			const RPoint2D& end);

	RPoint2D start();

	const RPoint2D& end();
	
protected:

	Line2D(const RPoint2D& a, const RPoint2D& b);

	RPoint2D m_end;

};

typedef Glib::RefPtr<Aeskulap::Line2D> RLine2D;

} // namespace Aeskulap

#endif // AESKULAP_LINE2D_H
